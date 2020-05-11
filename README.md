# StateMachineBot
Hello! This is my final project for my Embedded Systems course.

![The robot](https://github.com/awalm/StateMachineBot/blob/master/images/IMG_20200410_004002.jpg)

The sensors used on this robot are mainly an IR remote control, and an ultrasonic sensor.  The bot supports being directly controlled by the remote control, as well as recording and replaying movements. Hand gestures are detected using the ultrasonic sensor, and can be used to control the robot.

The brains of this robot is a 32 bit ARM Microcontroller, the LPC 804. The physical hardware of this robot is an arduino robot kit from Amazon.

A video demonstration of my bot in action can be found [here](https://www.youtube.com/watch?v=8Y4A216La6w&feature=youtu.be)

The source code can be found in the "src" folder.

Software Architecture Diagram:
![Software Architecture Diagram](https://github.com/awalm/StateMachineBot/blob/master/images/Software%20Architecture.png "Software Architecture Diagram")

Hardware Architecture Diagram:

![Hardware Architecture Diagram](https://github.com/awalm/StateMachineBot/blob/master/images/Schematic_updated.png "Hardware Architecture Diagram")
