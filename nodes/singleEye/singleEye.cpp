#include <iostream>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
// compiler takes care of showing us where these are
#include <DriveTrain.h>
#include <misc.h> // mapFloat(), mapDouble()

#include "main.h"

using namespace std;

uint64_t getTimeStamp(void ){
  timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

int main(int argc, char* argv[]) {

    char* DriveTrainMount = "/dev/ttyS0";

    DriveTrain drivetrain(DriveTrainMount);
    // anything in this range is automatically set to zero
    drivetrain.setWheelVelocityDeadZone(-100, 100);


    cout << "Setting watchdog timer...";
    drivetrain.setWatchdogTimer(0, true, true);
    cout << "done\n";

    uint64_t duration = 10 *1000000;
    uint64_t startTime = getTimeStamp();
    while(true){
       // send the mapped value to the motor controller
       drivetrain.wheelVelocity(500, robotRight, true, true);
       drivetrain.wheelVelocity(500, robotLeft, true, true);

    uint64_t currentTime = getTimeStamp();
    if( (currentTime - startTime) > duration){
	break;
     }
      sleep(.01);
    }
    //sleep(10); // ~3 secs

    drivetrain.wheelVelocity(500, robotLeft, true, true);
    drivetrain.wheelVelocity(500, robotRight, true, true);

    sleep(3);

    for(int i = 0; i < 3; i++)
        drivetrain.wheelHalt(true, true);

    return 0;
}
