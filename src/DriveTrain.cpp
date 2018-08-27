/*
    A driveTrain wrapper for the motor controller
*/

#include <RoboteqDevice.h>
#include <DriveTrain.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
// #include <unistd.h>
// #include <string.h>
#include <ErrorCodes.h>
#include <Constants.h>


DriveTrain::DriveTrain(char* fileSer) {
    this->fileName = fileSer;

    int status = device.Connect(fileName);

    if(status != RQ_SUCCESS)
    {
        std::cout<<"Error connecting to device: "<<status<<". " << strerror(errno) <<std::endl;
        exit(1) ;
    }
}

void DriveTrain::wheelVelocity(int velocity, RoboteqChannel w, bool getReturn, bool echoCmd) {

    if(this->deadBandCheck) {
        if(velocity > this->deadBandLowerBound && velocity < this->deadBandUpperBound){
            velocity = 0; // DEAD!
        }
    }
    int status = 0;

    switch(w) {
        case RoboteqChannel_1: {
            if(this->reverse_channel_1)
                velocity *= -1;
            // std::string cmd = "!G 1 " + std::to_string(velocity);
            if((status = device.SetCommand(_G, 1, velocity)) != RQ_SUCCESS){
                std::cout<<"failed --> "<<status<<std::endl;
                // this->sendGenericCommand(cmd, getReturn, echoCmd);
            }
            break;
        }
        case RoboteqChannel_2: {
            if(this->reverse_channel_2)
                velocity *= -1;
            if((status = device.SetCommand(_G, 2, velocity)) != RQ_SUCCESS){
                std::cout<<"failed --> "<<status<<std::endl;
            }
            // std::string cmd = "!G 2 " + std::to_string(velocity);
            // this->sendGenericCommand(cmd, getReturn, echoCmd);
            break;
        }
        default:
            this->wheelHalt();
            std::cerr << "ERROR in DriveTrain::wheelVelocity\n";
            return;
            break;
    }
}

void DriveTrain::wheelHalt(bool getReturn, bool echoCmd) {
    wheelVelocity(0, RoboteqChannel_1, getReturn, echoCmd);
    wheelVelocity(0, RoboteqChannel_2, getReturn, echoCmd);
}

void DriveTrain::setWatchdogTimer(int ms, bool getReturn, bool echoCmd) {
    this->sendGenericCommand("^RWD " + std::to_string(ms), getReturn, echoCmd);
}

void DriveTrain::factoryReset(bool getReturn, bool echoCmd) {
    this->sendGenericCommand("%EERST 321654987", getReturn, echoCmd);
}

void DriveTrain::sendGenericCommand(std::string cmd, bool getReturn, bool echoCmd) {
    if(echoCmd)
        std::cout << cmd << std::endl;

    cmd += this->cmd_term;
    mainSC.writeBuffer((char*)cmd.c_str(), cmd.length());

    // dont do this unless debugging
    if(getReturn) {
        char buf[1024];
        // reset the buffer
        for(int i = 0; i < 1024; i++)
            buf[i] = '\0'; // NULL

        mainSC.readBuffer(buf, 1023);        
        std::cout << "Response: " << buf << std::endl;
    }

}

void DriveTrain::setWheelVelocityDeadZone(int lowerBound, int upperBound) {
    if(lowerBound > upperBound) {
        std::cout << "Invalid bounds. Lower bound must be lower than upper bound\n";
    }

    this->deadBandCheck = true;
    this->deadBandLowerBound = lowerBound;
    this->deadBandUpperBound = upperBound;
}

void DriveTrain::disableMicroBasic(bool getReturn, bool echoCmd) {
    this->sendGenericCommand("^BRUN 0", getReturn, echoCmd);
}
