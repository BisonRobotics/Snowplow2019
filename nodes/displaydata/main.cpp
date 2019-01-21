#include <iostream>

// CPJL interface layer and 
// message type
#include <CPJL.hpp>
#include <cpp/XboxData.h>
#include <cpp/Encoder.h>
#include <cpp/SickMeasurement.h>
#include <cpp/ImuData.h>

// interface library for RoboteQ 

#include <misc.h>
#include <unistd.h>

using namespace std;

Encoder* encoder_data_rx = NULL;
XboxData* xbox_data_rx = NULL;
PathVector* path_vector = NULL;
ImuData* imu_data_rx = NULL;

uint64_t last_timestamp = -1;
const double setpoint = 50.0; // unit: RPM

void encoder_callback(void) 
{
    int left = encoder->left;
    int right = encoder->right;

    //send data to display
}

void xbox_callback(void) 
{
    int left_motor = xbox_data_rx->y_joystick_left;
    int right_motor = xbox_data_rx->y_joystick_right;

    left_motor = (int)mapFloat(left_motor, -32768, 32767, -1000, 1000);
    right_motor = (int)mapFloat(right_motor, -32768, 32767, -1000, 1000);

    //send data to display

    if(xbox_data_rx->button_b) {
        for(int i : {0, 1, 2})
            drive_train->wheelHalt(true, true);
        exit(EXIT_SUCCESS);
    }

    //cout << "Left: " << left_motor << ", Right: " << right_motor << endl;
}

void auto_callback(void)
{

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

    encoder = new Encoder(
        new CPJL("localhost", 14000),
        "encoder_data",
        encoder_callback
    );



    // start the asynch loop
    auto loop = CPJL_Message::loop();

    // sleep forever
    while(true)
        usleep(1000000);

    return 0;
}