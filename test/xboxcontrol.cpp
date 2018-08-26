#include <iostream>
#include <ctime>
#include <unistd.h>

// compiler takes care of showing us where these are
#include <XboxControllerInterface.h>
#include <DriveTrain.h>
// #include <.h>
#include <misc.h> // mapFloat(), mapDouble()

using namespace std;

int main(int argc, char* argv[]) {

    if(argc != 2) {
        cout << "Usage:\n  " << argv[0] << " <roboteq mount location>\n";
        exit(EXIT_FAILURE);
    }

    XboxController xbox;
    DriveTrain roboteq(argv[1]);

    // anything in this range is automatically set to zero
    roboteq.setWheelVelocityDeadZone(-100, 100);

    cout << "Disabling MicroBasic script...";
    roboteq.disableMicroBasic(true, true);
    cout << "done\n";

    cout << "Setting watchdog timer...";
    roboteq.setWatchdogTimer(0, true, true);
    cout << "done\n";

    bool running = true;
    while(running) {
        xbox.update();
        
        int inputR        = xbox.getJoyY(xBox_RIGHT);
        int mappedResultR = (int)mapFloat(inputR, -32768, 32767, -1000, 1000);

        int inputL = xbox.getJoyY(xBox_LEFT);
        int mappedResultL = (int)mapFloat(inputL, -32768, 32767, -1000, 1000);

        // send the mapped value to the motor controller
        roboteq.wheelVelocity(mappedResultR, RoboteqChannel_2, true, true);
        roboteq.wheelVelocity(mappedResultL, RoboteqChannel_1, true, true);

        // test B-button for stop
        if(xbox.buttonPressed(xBox_B))
            running = false;

        usleep(10000); // ~100x/sec        
    }

    for(int i = 0; i < 3; i++)
        roboteq.wheelHalt(true, true);

    return 0;
}
