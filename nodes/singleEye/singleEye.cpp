#include <iostream>
#include <ctime>
#include <unistd.h>

// compiler takes care of showing us where these are
#include <DriveTrain.h>
#include <misc.h> // mapFloat(), mapDouble()

#include "main.h"

using namespace std;

int main(int argc, char* argv[]) {

    char* DriveTrainMount = "/dev/ttyS0";

    DriveTrain drivetrain(DriveTrainMount);
    
    // anything in this range is automatically set to zero
    drivetrain.setWheelVelocityDeadZone(-100, 100);


    cout << "Setting watchdog timer...";
    drivetrain.setWatchdogTimer(0, true, true);
    cout << "done\n";        


    // send the mapped value to the motor controller
    drivetrain.wheelVelocity(500, robotRight, true, true);
    drivetrain.wheelVelocity(500, robotLeft, true, true);

    sleep(3); // ~3 secs

    drivetrain.wheelVelocity(500, robotLeft, true, true);
    drivetrain.wheelVelocity(500, robotRight, true, true);

    sleep(3);



    for(int i = 0; i < 3; i++)
        drivetrain.wheelHalt(true, true);

    return 0;
}
