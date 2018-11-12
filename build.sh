#!/bin/bash

# pretty text awaits
RED=`tput setaf 1`
GRN=`tput setaf 2`
YEL=`tput setaf 3`
MAG=`tput setaf 5`
CYN=`tput setaf 6`
RST=`tput sgr0`

print_command_line_opts()
{
    echo "${RED}  Options:"
    echo "    --lib   : (re)build all of the .o files needed for linking libraries"
    echo "    --bin   : (re)build all of the c++ executables (test files and final executable)"
    echo "    --vn    : (re)build all of the VectorNav libraries"
    echo "    --clean : remove all non-source/header files (.o .bin .exe)${RST}"
}

if [ $# -ne 1 ] # we need exactly one argument
then
    echo "${GRN}  Expecting only one argument"
    print_command_line_opts
    exit 128 # invalid argument
fi

# =======================================
# HERE BE DRAGONS
# =======================================

# some command line args for g++
STD_OPTS="-std=c++11 -march=native -O3 -Wall -lSDL"
INC_OPTS="-I./inc/" # the default directory for headers in this project

if [ $1 == --lib ] # build object code files
then
    echo "${YEL}  Building object (.o) files for linking..."

    # list of files that need to be compiled into object code
    LINK_FILES=( "serial-interface" "XboxControllerInterface" "RoboteqDevice" \
    "DriveTrain" "TCP_Connection" "SICK_Sensor" )

    for i in "${LINK_FILES[@]}" # iterate through the files
    do
        echo "${CYN}    src/$i.cpp"
        SRC="src/$i.cpp"
        LIB="lib/$i.o"

        STAT_SRC=`stat -c %Y $SRC`
        STAT_OUT=0

        if [ -e $LIB ] # make sure the .o file exists before testing it with stat
        then
            STAT_OUT=`stat -c %Y $LIB`
        fi

        if [ $STAT_SRC -gt $STAT_OUT ]
        then
            echo "      -- $SRC newer, recompiling..."
            g++ -c -o $LIB $SRC $STD_OPTS $INC_OPTS
        else
            echo "      -- $LIB newer, skipping compilation..."
        fi

    done

elif [ $1 == --vn ]
then

    echo "${YEL}  Building VectorNav libraries..."

    VNFILES=( `ls ./src/vn/` )
    echo "    files:${CYN}"

    for i in "${VNFILES[@]}"
    do
        SRC=${i%????}
        echo "      Compiling $i"
        #echo "        lib/$SRC.o"
        g++ -c -o ./lib/$SRC.o -w ./src/vn/$SRC.cpp $STD_OPTS $INC_OPTS
    done

elif [ $1 == --bin ] # build executables
then
    echo "${YEL}  Building executables..."

    echo "${CYN}    test/main.cpp"
    g++ test/main.cpp ./lib/DriveTrain.o ./lib/serial-interface.o ./lib/RoboteqDevice.o -o bin/main \
    $STD_OPTS $INC_OPTS

    echo "${CYN}    test/xboxcontrol.cpp"
    g++ test/xboxcontrol.cpp ./lib/DriveTrain.o ./lib/XboxControllerInterface.o ./lib/RoboteqDevice.o \
    ./lib/serial-interface.o -o bin/xboxcontrol $STD_OPTS $INC_OPTS

    echo "${CYN}    test/motorcontrollerTest.cpp"
    g++ test/motorcontrollerTest.cpp ./lib/DriveTrain.o  ./lib/serial-interface.o ./lib/RoboteqDevice.o \
    -o bin/motorcontrollerTest $STD_OPTS $INC_OPTS

    echo "${CYN}    test/sicksensormsgsplit.cpp"
    g++ test/sicksensormsgsplit.cpp ./lib/TCP_Connection.o ./lib/SICK_Sensor.o -o bin/sicksensormsgsplit \
    $STD_OPTS $INC_OPTS

    echo "${CYN}  app/teleop"
    g++ app/teleop.cpp ./lib/DriveTrain.o ./lib/serial-interface.o ./lib/XboxControllerInterface.o ./lib/RoboteqDevice.o \
    -o bin/teleop $STD_OPTS $INC_OPTS


elif [ $1 == --clean ] # delete all unneccessary files
then
    echo "${YEL}  Cleaning up the workspace..."

    # remove everything in bin/ and lib/
    rm -rf bin/*
    rm -rf lib/*

fi

# i never finish anythi
