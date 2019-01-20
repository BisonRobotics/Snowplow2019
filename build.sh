#!/bin/bash

# pretty text awaits
RED=`tput setaf 1`
GRN=`tput setaf 2`
YEL=`tput setaf 3`
MAG=`tput setaf 5`
CYN=`tput setaf 6`
RST=`tput sgr0`

# find the MRM timestamp for the given file. 
# if file does not exist, returns zero
file_timestamp() # file_timestamp fileToTest resultReturn
{
    if [ -e $1 ]; then
        local ts=`stat -c %Y $1`
        eval $2=$ts
    else
        eval $2=0
    fi
}

print_command_line_opts()
{
    echo "${RED}  Options:"
    echo "    --all   : perform all of the steps below, starting with --clean"
    echo "    --lib   : (re)build all of the .o files needed for linking libraries, but no VN libs"
    echo "    --bin   : (re)build all of the c++ executables (test files only)"
    echo "    --vn    : (re)build all of the VectorNav libraries"
    echo "    --nodes : (re)build all of the nodes for the final program"
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

# create certain folders if they dont exist
FOLDERS=( "lib" "bin" "lib/vn" )
for foldername in "${FOLDERS[@]}"
do
    if [ ! -d $foldername ]; then
        echo "${MAG}Creating directory '$foldername'${RST}"
        mkdir $foldername
    fi
done

# some command line args for g++
STD_OPTS="-std=c++11 -march=native -O3 -Wall -lSDL"
INC_OPTS="-I./inc/" # the default directory for headers in this project

if [ $1 == --all ]
then
    printf "${RED}This will delete all existing binaries. Are you sure you want to continue [y/n] "
    read -n1 reply
    echo "${RST}"

    if [ "$reply" != "y" ] && [ "$reply" != "Y" ]
    then
        echo "Exiting..."
        exit 128
    fi

    echo "${YEL}  Building everything..."
    bash build.sh --clean
    bash build.sh --lib
    bash build.sh --vn
    bash build.sh --bin
    bash build.sh --nodes

elif [ $1 == --lib ] # build object code files
then
    echo "${YEL}  Building object (.o) files for linking..."

    LINK_FILES=( `ls ./src` )
    for fname in "${LINK_FILES[@]}" # iterate through the files
    do
        if [ -f "./src/$fname" ]
        then
            i=${fname%????}

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
                printf "      Source newer, recompiling..."
                g++ -c -o $LIB $SRC $STD_OPTS $INC_OPTS
                echo "${GRN}DONE"
            else
                echo "      Object file newer, skipping compilation..."
            fi
        fi
    done

elif [ $1 == --nodes ]
then

    NODEFILES=( `ls ./nodes/` )
    for folder in "${NODEFILES[@]}"
    do
        if [ -d ./nodes/$folder ]
        then
            # this is a directory, assume it contains a project node
            echo "${YEL}  Compiling ./nodes/$folder ${RST}"
            cd ./nodes/$folder && bash build.sh
            cd ../..
        fi
    done

elif [ $1 == --vn ]
then

    echo "${YEL}  Building VectorNav libraries..."

    VNFILES=( `ls ./src/vn/` )
    echo "    files:${CYN}"

    for i in "${VNFILES[@]}"
    do
        echo "      ${CYN}./src/vn/$i"
        SRC=${i%????}

        # do a quick timestamp check, second 
        # arg is where result is stored
        file_timestamp "./src/vn/$SRC.cpp" ORIG
        file_timestamp "./lib/vn/$SRC.o" OBJ

        if [ $ORIG -gt $OBJ ]; then
            printf "        ${CYN}Source file newer, compiling..."
            g++ -c -o ./lib/vn/$SRC.o -w ./src/vn/$SRC.cpp $STD_OPTS $INC_OPTS
            echo "${GRN}DONE"
        else
            echo "        ${CYN}Object file newer, skipping compilation"        
        fi

        #echo "      Compiling $i"
        #g++ -c -o ./lib/vn/$SRC.o -w ./src/vn/$SRC.cpp $STD_OPTS $INC_OPTS
    done

elif [ $1 == --bin ] # build executables
then
    echo "${YEL}  Building executables..."

    echo "${CYN}    test/main.cpp"
    g++ test/main.cpp ./lib/DriveTrain.o ./lib/serial-interface.o ./lib/RoboteqDevice.o -o bin/main.exe \
    $STD_OPTS $INC_OPTS

    echo "${CYN}    test/xboxcontrol.cpp"
    g++ test/xboxcontrol.cpp ./lib/DriveTrain.o ./lib/XboxControllerInterface.o ./lib/RoboteqDevice.o \
    ./lib/serial-interface.o -o bin/xboxcontrol.exe $STD_OPTS $INC_OPTS

    echo "${CYN}    test/motorcontrollerTest.cpp"
    g++ test/motorcontrollerTest.cpp ./lib/DriveTrain.o  ./lib/serial-interface.o ./lib/RoboteqDevice.o \
    -o bin/motorcontrollerTest.exe $STD_OPTS $INC_OPTS

    echo "${CYN}    test/sicksensormsgsplit.cpp"
    g++ test/sicksensormsgsplit.cpp ./lib/TCP_Connection.o ./lib/SICK_Sensor.o -o bin/sicksensormsgsplit.exe \
    $STD_OPTS $INC_OPTS

    #echo "${CYN}  app/teleop"
    #g++ app/teleop.cpp ./lib/DriveTrain.o ./lib/serial-interface.o ./lib/XboxControllerInterface.o ./lib/RoboteqDevice.o \
    #-o bin/teleop.exe $STD_OPTS $INC_OPTS


elif [ $1 == --clean ] # delete all unneccessary files
then
    echo "${YEL}  Cleaning up the workspace..."

    # remove everything in bin/ and lib/
    rm -rf bin/*
    rm -rf lib/*
    rmdir bin
    rmdir lib

    # remove object code files and executables in nodes/*
    NODEFILES=( `ls ./nodes/` )
    for folder in "${NODEFILES[@]}"
    do
        FILDES=( "main.exe" "main.out" "main.o" )
        for filename in "${FILDES[@]}"
        do
            if [ -e ./nodes/$folder/$filename ]
            then
                # not testing if this exists first can cause 'incidents'
                rm ./nodes/$folder/$filename
            fi
        done
    done

fi
