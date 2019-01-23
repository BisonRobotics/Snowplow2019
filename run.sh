#!/bin/bash

#start cpjl

./../CPJL/lib/server/v3/main -addr localhost -port 14000 >> cpjl.log &&

#start encode node
python3 /nodes/encoderData/ main.py >> enncoders.log &&

#start motorcontroller
./nodes/motorcontrol/main.exe /dev/ttyS0 >> motorcontrol.log &&

# start pathplanner
./nodes/pathplanner/main.exe TELEOP &&


