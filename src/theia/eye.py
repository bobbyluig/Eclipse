import numpy as np
import cv2
import logging
import urllib.request
from collections import deque

logger = logging.getLogger('universe')


class IPCamera:
    def __init__(self, url):
        self.stream = urllib.request.urlopen(url)
        self.b = bytearray()

    def read(self):
        max = 1000
        while max > 0:
            self.b.extend(self.stream.read(1024))
            x = self.b.find(b'\xff\xd8')
            y = self.b.find(b'\xff\xd9')

            max -= 1

            if x != -1 and y != -1:
                jpg = self.b[x:y+2]
                self.b = self.b[y+2:]

                jpg = np.asarray(jpg, dtype=np.uint8)
                i = cv2.imdecode(jpg, cv2.IMREAD_UNCHANGED)

                return True, i


class Camera:
    def __init__(self, source, fx, fy, size=(640, 480)):
        self.source = source
        self.fx = fx
        self.fy = fy
        self.width, self.height = size


class Eye:
    def __init__(self, camera, debug=True):
        self.history = deque(maxlen=5)
        self.frame = None
        self.debug = debug

        source = camera.source
        self.cap = cv2.VideoCapture(source)

        if not self.cap.isOpened():
            raise ConnectionError('Unable to connect to video source "{}".'.format(source))

        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, camera.width)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, camera.height)

        self.width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        self.height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

        if self.width != camera.width or self.height != camera.height:
            raise ConnectionError('Unable to open video source at {:d} x {:d}.'
                                  .format(source, camera.width, camera.height))

    def close(self):
        self.cap.release()

    def update_frame(self):
        if self.frame is not None:
            self.history.appendleft(self.frame.copy())

        _, self.frame = self.cap.read()

        if self.debug:
            cv2.imshow('debug', self.frame)
            cv2.waitKey(1)

    def get_gray_frame(self):
        self.update_frame()
        return cv2.cvtColor(self.frame, cv2.COLOR_BGR2GRAY)

    def get_color_frame(self):
        self.update_frame()
        return self.frame.copy()

    def get_flipped_frame(self):
        frame = self.get_color_frame()
        return cv2.flip(frame, flipCode=-1)

    def get_both_frames(self):
        self.update_frame()
        return self.frame.copy(), cv2.cvtColor(self.frame, cv2.COLOR_BGR2GRAY)
