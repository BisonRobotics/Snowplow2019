#!/bin/bash

INC_OPTS="-I./../../../CPJL/lib/cpp/ -I./../../../CPJL/lib/gen/messages/ -I./../../inc/"
STD_OPTS="-std=c++11 -march=native -O3 -ffast-math -lpthread -Wall -fno-strict-aliasing -lSDL"
OBJ="./../../lib/SICK_Sensor.o ./../../lib/TCP_Connection.o"

g++ -o main.exe main.cpp ${OBJ} ${INC_OPTS} ${STD_OPTS}
