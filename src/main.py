#!/usr/bin/env python3.5

import logging
import config
import asyncio
import ssl

from autobahn.asyncio.wamp import ApplicationSession, ApplicationRunner
from autobahn.wamp import auth
# from cerebral.autoreconnect import ApplicationRunner

##############################
# Create the main application.
##############################

logger = logging.getLogger('universe')

# Constants.
user = 'DOG-1E5'
password = 'de2432k,/s-=/8Eu'


class Cerebral(ApplicationSession):
    def onConnect(self):
        logger.info('Connected to server.')
        self.join(self.config.realm, ['wampcra'], user)

    def onChallenge(self, challenge):
        logger.info('Challenge received.')
        if challenge.method == 'wampcra':
            if 'salt' in challenge.extra:
                key = auth.derive_key(password.encode(),
                                      challenge.extra['salt'].encode(),
                                      challenge.extra.get('iterations', None),
                                      challenge.extra.get('keylen', None))
            else:
                key = password.encode()
            signature = auth.compute_wcs(key, challenge.extra['challenge'])
            return signature.decode('ascii')
        else:
            raise Exception('Unknown challenge method: %s' % challenge.method)

    def onJoin(self, details):
        logger.info('Joined "%s" realm.' % self.config.realm)
        self.register(self.identify, 'dog.identify')
        self.register(self.hello, 'dog.hello')

    def onDisconnect(self):
        logger.warning('Connection lost!')

    ############
    # Functions.
    ############

    def identify(self):
        self.call('zeus.speak', "Hello. I am DOG-1E5, Eclipse Technology's first generation quadruped. "
                                "I am designed for Project Lycanthrope by E D D Red Team 2016. "
                                "Rawr."
                  )

    def hello(self):
        self.call('zeus.speak', "Hello world!")

if __name__ == '__main__':
    ip = '192.168.0.6'

    context = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
    context.verify_mode = ssl.CERT_REQUIRED
    context.check_hostname = False
    pem = ssl.get_server_certificate((ip, 443))
    context.load_verify_locations(cadata=pem)

    runner = ApplicationRunner(url='wss://%s/ws' % ip, realm='lycanthrope',
                               ssl=context)

    runner.run(Cerebral)
