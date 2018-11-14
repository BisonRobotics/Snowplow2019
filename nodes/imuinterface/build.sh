#!/bin/bash

INC_OPTS="-I./../../../CPJL/lib/cpp/ -I./../../../CPJL/lib/gen/messages/ -I./../../inc/"
STD_OPTS="-std=c++11 -march=native -O3 -ffast-math -lpthread -Wall -fno-strict-aliasing -lSDL"

VN_LINK_FILES=( `ls ./../../lib/vn/` )
LINK_STR=""
for file in "${VN_LINK_FILES[@]}"
do
    # build up the link string
    LINK_STR="$LINK_STR ./../../lib/vn/$file"
done

g++ -c -o main.o main.cpp ${INC_OPTS} ${STD_OPTS}
g++ -o main.exe main.o ${LINK_STR} ${INC_OPTS} ${STD_OPTS}
