#!/bin/bash

INC_OPTS="-I./../../../CPJL/lib/cpp/ -I./../../../CPJL/lib/gen/messages/"
STD_OPTS="-std=c++11 -march=native -O3 -ffast-math -lpthread -Wall -fno-strict-aliasing"

g++ -o main main.cpp ${INC_OPTS} ${STD_OPTS}
