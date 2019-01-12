#include <iostream>

// CPJL interface layer and 
// message type
#include <CPJL.hpp>
#include <cpp/XboxData.h>

// interface library for RoboteQ 
// motor controller
#include <DriveTrain.h>
#include <misc.h>

#include <unistd.h>

using namespace std;

XboxData* xbox_data_rx = NULL;
DriveTrain* dt = NULL;

void callback(void) {
    int left_motor 
        = xbox_data_rx->y_joystick_left;
    int right_motor 
        = xbox_data_rx->y_joystick_right;

    left_motor = (int)mapFloat(left_motor, -32768, 32767, -1000, 1000);
    right_motor = (int)mapFloat(right_motor, -32768, 32767, -1000, 1000);

    dt->wheelVelocity(right_motor, RoboteqChannel_2, true, true);
    dt->wheelVelocity(left_motor, RoboteqChannel_1, true, true);

    if(xbox_data_rx->button_b) {
        for(int i : {0, 1, 2})
            dt->wheelHalt(true, true);
        exit(EXIT_SUCCESS);
    }

    cout << "Left: " << left_motor << ", Right: " << right_motor << endl;
}

int main(int argc, char* argv[]) {

    if(argc != 2) {
        cout << "Usage:\n  " << argv[0] << " <roboteq mount location>\n";
        exit(EXIT_FAILURE);
    }

    xbox_data_rx = new XboxData(
        new CPJL("localhost", 14000), 
        "xbox_data",
        callback
    );

    dt = new DriveTrain(argv[1]);

    // start the asynch loop
    auto loop = CPJL_Message::loop();

    // sleep forever
    while(true)
        usleep(1000000);

    return 0;
}