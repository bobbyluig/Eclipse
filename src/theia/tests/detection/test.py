from oculus import Line2DParameters, Line2D
import cv2
import time


p = Line2DParameters()
p.useGaussian = True
line = Line2D(p)


def cam():
    camera = cv2.VideoCapture(0)

    while True:
        _, frame = camera.read()
        line.test(frame)
        cv2.waitKey(1)


def image():
    frame = cv2.imread('roi.jpg')
    line.test(frame)
    cv2.waitKey(0)


image()