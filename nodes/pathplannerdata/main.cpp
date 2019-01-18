#include <iostream>

// CPJL interface layer and 
// message type
#include <CPJL.hpp>
#include <cpp/XboxData.h>
#include <cpp/Encoder.h>
#include <cpp/MotorControlCommand.h>
#include <cpp/ImuData.h>

#include <misc.h>
#include <unistd.h>

using namespace std;

XboxData* xbox_data_rx = NULL;
MotorControlCommand* motor_control_command = NULL;
ImuData* imu_data_rx = NULL;

uint64_t last_timestamp = -1;


void xbox_callback(void) {
    //receive data in xbox
    int left_motor 
        = xbox_data_rx->y_joystick_left;
    int right_motor 
        = xbox_data_rx->y_joystick_right;

    left_motor = (int)mapFloat(left_motor, -32768, 32767, -50, 50);
    right_motor = (int)mapFloat(right_motor, -32768, 32767, -50, 50);

    if(xbox_data_rx->button_b) 
    {
        for(int i : {0, 1, 2})
        {
            motor_control_command->left = 0;
            motor_control_command->right = 0;
            motor_control_command->putMessage();
        }
        exit(EXIT_SUCCESS);
    }

    //package xbox data into motor controller command
    motor_control_command->left = left_motor;
    motor_control_command->right = right_motor;
    motor_control_command->putMessage();

    //cout << "Left: " << left_motor << ", Right: " << right_motor << endl;
}

void imu_callback(void) {
    //receive data from IMU
    float x_acc 
        = imu_data_rx->x_acc;
    float y_acc 
        = imu_data_rx->y_acc;
    float x_vel 
        = imu_data_rx->x_vel;
    float y_vel 
        = imu_data_rx->y_vel;
    float z_orient 
        = imu_data_rx->z_orient;

    //?????

    
}

int main(int argc, char* argv[]) {

    /*if(argc != 2) {
        cout << "Usage:\n  " << argv[0] << " <roboteq mount location>\n";
        exit(EXIT_FAILURE);
    }*/

    //ADD - get priority for receiving data from the command arguments

    xbox_data_rx = new XboxData(
        new CPJL("localhost", 14000), 
        "xbox_data",
        xbox_callback
    );

    // start the asynch loop
    auto loop = CPJL_Message::loop();

    // sleep forever
    while(true)
        usleep(1000000);

    return 0;
}
