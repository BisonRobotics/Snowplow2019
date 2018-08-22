#ifndef __JJC__ROBOTEQ__H__
#define __JJC__ROBOTEQ__H__

#include <iostream>
#include <serial-interface.h>

enum RoboteQChannel {
    RoboteQChannel_1, RoboteQChannel_2
};

class RoboteQInterface {
private:
    // serial interface used internally by RoboteQInterface
    SerialController mainSC;

    // RoboteQ can be reached by reading/writing this file
    char* fileName;

    char cmd_term = '_';

    // flags indicating whether a particular channel should be reversed
    bool reverse_channel_1 = false;
    bool reverse_channel_2 = false;

    // get return flags, ignore args passed via method call
    bool _getReturn = false;
    bool _echoCmd = false;

    bool deadBandCheck = false;
    int deadBandUpperBound = 0;
    int deadBandLowerBound = 0;

public:
    // constructor given a
    RoboteQInterface(char* fileSer);

    RoboteQInterface& channelRev(RoboteQChannel ch, bool rev);    

    // set the corresponding wheel to a given velocity
    // -1000 -> +1000
    void wheelVelocity(int velocity, RoboteQChannel w, bool getReturn = false, bool echoCmd = false);

    // bring speed of both wheels to zero immediately
    void wheelHalt(bool getReturn = false, bool echoCmd = false);

    // set watchdog timer to the specified number of milliseconds
    // set to zero to reset
    void setWatchdogTimer(int ms, bool getReturn = false, bool echoCmd = false);

    // factory reset the motor controller
    void factoryReset(bool getReturn = false, bool echoCmd = false);

    // for commands not yet part of the library
    void sendGenericCommand(std::string cmd, bool getReturn = false, bool echoCmd = false);

    // set a dead-zone for wheelVelocity commands
    void setWheelVelocityDeadZone(int lowerBound, int upperBound);

    // disable the MicroBasic script
    void disableMicroBasic(bool getReturn = false, bool echoCmd = false);

};

#endif // __JJC__ROBOTEQ__H__
