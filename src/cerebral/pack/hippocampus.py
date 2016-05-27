from agility.main import Servo, Leg, Robot, Head, Body
from finesse.eclipse import Finesse
from theia.eye import Camera


################
# Define robot.
################

class Android:
    # Camera. 'http://127.0.0.1:8080/?action=stream&dummy.mjpg'
    camera = Camera('http://127.0.0.1:8080/?action=stream&dummy.mjpg', 90, 60)

    # Leg 1.
    servo1 = Servo(0, -180, 90, 500, 2500, 150, bias=-10, direction=1)
    servo2 = Servo(1, -45, 225, 500, 2500, 150, bias=4, direction=1)
    servo3 = Servo(2, -135, 135, 500, 2500, 150, bias=7, direction=-1)
    leg1 = Leg(servo1, servo2, servo3, (6.4, 7.25), 0, Finesse.inverse_pack, Finesse.forward_pack)

    # Leg 2.
    servo4 = Servo(3, -90, 180, 500, 2500, 150, bias=10, direction=-1)
    servo5 = Servo(4, -225, 45, 500, 2500, 150, bias=4, direction=1)
    servo6 = Servo(5, -135, 135, 500, 2500, 150, bias=4, direction=1)
    leg2 = Leg(servo4, servo5, servo6, (6.4, 7.25), 1, Finesse.inverse_pack, Finesse.forward_pack)

    # Leg 3.
    servo7 = Servo(6, -90, 180, 500, 2500, 150, bias=-6, direction=1)
    servo8 = Servo(7, -45, 225, 500, 2500, 150, bias=-5, direction=1)
    servo9 = Servo(8, -135, 135, 500, 2500, 150, bias=3, direction=-1)
    leg3 = Leg(servo7, servo8, servo9, (6.4, 7.25), 2, Finesse.inverse_pack, Finesse.forward_pack)

    # Leg 4 .
    servo10 = Servo(9, -180, 90, 500, 2500, 150, bias=-7, direction=-1)
    servo11 = Servo(10, -225, 45, 500, 2500, 150, bias=-5, direction=1)
    servo12 = Servo(11, -135, 135, 500, 2500, 150, bias=0, direction=1)
    leg4 = Leg(servo10, servo11, servo12, (6.4, 7.25), 3, Finesse.inverse_pack, Finesse.forward_pack)

    # Head
    servo16 = Servo(16, -90, 90, 400, 2400, 100, bias=8, direction=1, left_bound=-45, right_bound=45)
    servo17 = Servo(17, -90, 90, 400, 2400, 100, bias=3, direction=1, left_bound=-20, right_bound=20)
    head = Head(servo16, servo17, camera)

    # Body
    body = Body(length=16.7, width=14.8, cx=-0.3, cy=0.1, mb=20, ml=0)

    # Robot.
    robot = Robot(leg1, leg2, leg3, leg4, body, head)

    # Information.
    info = {
        'max_speed': 10,            # Max forward speed in cm/sec.
        'max_rotation': 1.0472      # Max rotation speed in rads/sec.
    }


#######################
# Connection variables.
#######################

class Crossbar:
    # Crossbar.
    ip = '192.168.56.1'
    realm = 'lycanthrope'
    authid = 'DOG-4S1'
    secret = 'A@Q+xfQ[9<<5,+YG'
    prefix = 'pack2'


#############################
# Basic conversation topics.
#############################

class Conversation:
    topics = {
        'hello': "Hello World!",
        'identify': "Hello. I am DOG-1E5, Eclipse Technology's first generation quadruped. "
                    "I am designed for Project Lycanthrope by E D D Red Team 2016. "
                    "Rawr.",
        'introduce': "Hello. I am DOG-1E5 from Eclipse Technologies. Welcome to our Proof of Concept.",
        'how are you': "I am very nervous. I hope I am doing everything correctly."
    }


