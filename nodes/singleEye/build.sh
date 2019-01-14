#!/bin/bash

INC_OPTS="-I./../../../CPJL/lib/cpp/ -I./../../../CPJL/lib/gen/messages/ -I./../../inc"
STD_OPTS="-std=c++11 -march=native -O3 -ffast-math -lpthread -Wall -fno-strict-aliasing"

g++ -o singleEye.exe singleEye.cpp ${INC_OPTS} ${STD_OPTS} ./../../lib/serial-interface.o ./../../lib/DriveTrain.o ./../../lib/RoboteqDevice.o
