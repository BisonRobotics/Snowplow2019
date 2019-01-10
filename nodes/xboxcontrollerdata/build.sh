#!/bin/bash

INC_OPTS="-I./../../../CPJL/lib/cpp/ -I./../../../CPJL/lib/gen/messages/ -I./../../inc/"
STD_OPTS="-std=c++11 -march=native -O3 -ffast-math -lpthread -Wall -fno-strict-aliasing -lSDL"

g++ -o main.exe main.cpp ./../../lib/XboxControllerInterface.o ${INC_OPTS} ${STD_OPTS}
