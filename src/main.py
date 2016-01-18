#!/usr/bin/env python3.5

import asyncio, logging, time

from autobahn.asyncio.wamp import ApplicationSession
from autobahn.wamp import auth
from cerebral.autoreconnect import ApplicationRunner

####################
# Configure logging.
####################

logger = logging.getLogger('universe')
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)
formatter = logging.Formatter(fmt='%(asctime)s | %(levelname)s | %(message)s', datefmt='%m/%d/%Y %H:%M:%S')
ch.setFormatter(formatter)
logger.addHandler(ch)

############
# Functions.
############





##############################
# Create the main application.
##############################

# Constants.
user = 'DOG-1E5'
password = 'de2432k,/s-=/8Eu'


class Cerebral(ApplicationSession):
    def onConnect(self):
        logging.info('Connected to server.')
        self.join(self.config.realm, ['wampcra'], user)

    def onChallenge(self, challenge):
        logging.info('Challenge received.')
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

    async def onJoin(self, details):
        logging.info('Joined "%s" realm.' % self.config.realm)
        await self.register(moveServo, 'com.agility')

    def onDisconnect(self):
        logging.warning('Connection lost!')

if __name__ == '__main__':
    ip = '127.0.0.1'
    runner = ApplicationRunner(url='ws://%s:8080/ws' % ip, realm='lycanthrope')
    runner.run(Cerebral)
