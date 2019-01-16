#include <iostream>

// CPJL interface layer and 
// message type
#include <CPJL.hpp>
#include <cpp/XboxData.h>
#include <cpp/Encoder.h>

// interface library for RoboteQ 
// motor controller
#include <DriveTrain.h>

#include <misc.h>
#include <unistd.h>

using namespace std;

XboxData* xbox_data_rx = NULL;
MotorControlCommand* motor_control_command = NULL;

uint64_t last_timestamp = -1;


void xbox_callback(void) {
    //receive data in xbox
    int left_motor
        = xbox_data_rx->y_joystick_left;
    int right_motor 
        = xbox_data_rx->y_joystick_right;

    left_motor = (int)mapFloat(left_motor, -32768, 32767, -1000, 1000);
    right_motor = (int)mapFloat(right_motor, -32768, 32767, -1000, 1000);

    if(xbox_data_rx->button_b) {
        for(int i : {0, 1, 2})
            motor_control_command->left = 0;
            motor_control_command->right = 0;
            motor_control_command->putMessage();
        exit(EXIT_SUCCESS);
    }

    //package xbox data into motor controller command
    motor_control_command->left = left_motor;
    motor_control_command->right = right_motor;
    motor_control_command->putMessage();

    //cout << "Left: " << left_motor << ", Right: " << right_motor << endl;
}

int main(int argc, char* argv[]) {

    if(argc != 2) {
        cout << "Usage:\n  " << argv[0] << " <roboteq mount location>\n";
        exit(EXIT_FAILURE);
    }

    //ADD - get priority for receiving data from the command arguments

    xbox_data_rx = new XboxData(
        new CPJL("localhost", 14000), 
        "xbox_data",
        callback
    );

    // start the asynch loop
    auto loop = CPJL_Message::loop();

    // sleep forever
    while(true)
        usleep(1000000);

    return 0;
}
