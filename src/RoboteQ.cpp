/*
    Implementation of RoboteQ interface
    Model: VDC2450
*/

#include <iostream>
#include <RoboteQ.h>
#include <serial-interface.h>
#include <string>
#include <string.h>

RoboteQInterface::RoboteQInterface(char* fileSer) {
    this->fileName = fileSer;

    // initialize serial interface for motor controller
    mainSC.set_SerialPort(fileSer);
    mainSC.set_BaudRate(BaudRate_115200);
    mainSC.set_Parity(Parity_NONE);
    mainSC.set_StopBits(StopBits_1);
    mainSC.set_WordSize(WordSize_8);
    mainSC.start();
}

void RoboteQInterface::wheelVelocity(int velocity, RoboteQChannel w, bool getReturn, bool echoCmd) {

    if(this->deadBandCheck) {
        if(velocity > this->deadBandLowerBound && velocity < this->deadBandUpperBound)
            velocity = 0; // DEAD!
    }

    switch(w) {
        case RoboteQChannel_1: {
            if(this->reverse_channel_1)
                velocity *= -1;
            std::string cmd = "!G 1 " + std::to_string(velocity);
            this->sendGenericCommand(cmd, getReturn, echoCmd);
            break;
        }
        case RoboteQChannel_2: {
            if(this->reverse_channel_2)
                velocity *= -1;
            std::string cmd = "!G 2 " + std::to_string(velocity);
            this->sendGenericCommand(cmd, getReturn, echoCmd);
            break;
        }
        default:
            this->wheelHalt();
            std::cerr << "ERROR in RoboteQInterface::wheelVelocity\n";
            return;
            break;
    }
}

void RoboteQInterface::wheelHalt(bool getReturn, bool echoCmd) {
    wheelVelocity(0, RoboteQChannel_1, getReturn, echoCmd);
    wheelVelocity(0, RoboteQChannel_2, getReturn, echoCmd);
}

void RoboteQInterface::setWatchdogTimer(int ms, bool getReturn, bool echoCmd) {
    this->sendGenericCommand("^RWD " + std::to_string(ms), getReturn, echoCmd);
}

void RoboteQInterface::factoryReset(bool getReturn, bool echoCmd) {
    this->sendGenericCommand("%EERST 321654987", getReturn, echoCmd);
}

void RoboteQInterface::sendGenericCommand(std::string cmd, bool getReturn, bool echoCmd) {
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

void RoboteQInterface::setWheelVelocityDeadZone(int lowerBound, int upperBound) {
    if(lowerBound > upperBound) {
        std::cout << "Invalid bounds. Lower bound must be lower than upper bound\n";
    }

    this->deadBandCheck = true;
    this->deadBandLowerBound = lowerBound;
    this->deadBandUpperBound = upperBound;
}

void RoboteQInterface::disableMicroBasic(bool getReturn, bool echoCmd) {
    this->sendGenericCommand("^BRUN 0", getReturn, echoCmd);
}
