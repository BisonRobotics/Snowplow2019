#include <iostream>
#include <ctime>
#include <unistd.h>

// compiler takes care of showing us where these are
#include <XboxControllerInterface.h>
#include <DriveTrain.h>
#include <RoboteqDevice.h>

#include <misc.h> // mapFloat(), mapDouble()

using namespace std;

int main(int argc, char* argv[]) {

    if(argc != 2) {
        cout << "Usage:\n  " << argv[0] << " <plow MC mount Location>\n";
        exit(EXIT_FAILURE);
    }

    char* DriveTrainMount;
    DriveTrainMount = "/dev/ttyS0";

    XboxController xbox;
    DriveTrain drivetrain(DriveTrainMount);
    RoboteqDevice plow;
    plow.Connect(argv[1]); // should be something like /dev/ttyUSB0

    // anything in this range is automatically set to zero
    drivetrain.setWheelVelocityDeadZone(-100, 100);


    cout << "Setting watchdog timer...";
    drivetrain.setWatchdogTimer(0, true, true);
    cout << "done\n";

    int status;
    int plowSpeed = 500;

    bool running = true;
    while(running) {
        xbox.update();

        int inputR        = xbox.getJoyY(xBox_RIGHT);
        int mappedResultR = (int)mapFloat(inputR, -32768, 32767, -1000, 1000);

        int inputL = xbox.getJoyY(xBox_LEFT);
        int mappedResultL = (int)mapFloat(inputL, -32768, 32767, -1000, 1000);

        // send the mapped value to the motor controller
        drivetrain.wheelVelocity(mappedResultR, RoboteqChannel_2, true, true);
        drivetrain.wheelVelocity(mappedResultL, RoboteqChannel_1, true, true);

        // test B-button for stop
        if(xbox.buttonPressed(xBox_B)){
            running = false;
        }

        if (xbox.buttonPressed(xBox_A)){
            if((status = plow.SetCommand(_G, 1, plowSpeed)) != RQ_SUCCESS){
                std::cout<<"failed --> "<<status<<std::endl;
            }
        }else if(xbox.buttonPressed(xBox_X)){
            if((status = plow.SetCommand(_G, 1, -plowSpeed)) != RQ_SUCCESS){
                std::cout<<"failed --> "<<status<<std::endl;
            }
        }else{
            if((status = plow.SetCommand(_G, 1, 0)) != RQ_SUCCESS){
                std::cout<<"failed --> "<<status<<std::endl;
            }
        }

        usleep(10000); // ~100x/sec
    }

    for(int i = 0; i < 3; i++)
        drivetrain.wheelHalt(true, true);

    return 0;
}
