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
    echo "    --lib : (re)build all of the .o files needed for linking libraries"
    echo "    --bin : (re)build all of the c++ executables (test files and final executable)${RST}"
}

if [ $# -gt 1 ] || [ $# -lt 1 ] # we need exactly one argument
then
    echo "${GRN}  Expecting only one argument"
    print_command_line_opts
    exit 128 # invalid argument
fi

# =======================================
# HERE BE DRAGONS
# =======================================

# some command line args for g++
STD_OPTS="-std=c++11 -march=native -O3 -Wall"
INC_OPTS="-I./inc/" # the default directory for headers in this project

if [ $1 == --lib ] # build object code files
then
    echo "${YEL}  Building object (.o) files for linking..."

    LINK_FILES=( "serial-interface" ) # list of files that need to be compiled into object code
    for i in "${LINK_FILES[@]}"
    do
        echo "${CYN}    src/$i.cpp"
        SRC="src/$i.cpp"
	LIB="-c -o lib/$i.o"

	# compile command
        g++ $LIB $SRC $STD_OPTS $INC_OPTS

    done
fi

echo "${MAG}  Building sample hello world..."
RESULT=`g++ src/main.cpp -o bin/main  $STD_OPTS $INC_OPTS`

# i never finish anythi
