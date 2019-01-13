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

Encoder* encoder = NULL;
XboxData* xbox_data_rx = NULL;
DriveTrain* drive_train = NULL;

void callback(void) {
    int left_motor 
        = xbox_data_rx->y_joystick_left;
    int right_motor 
        = xbox_data_rx->y_joystick_right;

    left_motor = (int)mapFloat(left_motor, -32768, 32767, -1000, 1000);
    right_motor = (int)mapFloat(right_motor, -32768, 32767, -1000, 1000);

    drive_train->wheelVelocity(right_motor, RoboteqChannel_2, true, true);
    drive_train->wheelVelocity(left_motor, RoboteqChannel_1, true, true);

    if(xbox_data_rx->button_b) {
        for(int i : {0, 1, 2})
            drive_train->wheelHalt(true, true);
        exit(EXIT_SUCCESS);
    }

    //cout << "Left: " << left_motor << ", Right: " << right_motor << endl;
}

uint64_t last_timestamp = -1;
const double setpoint = 1.0;
double left_integral = 0.0;

void encoder_callback(void) {
    double dt = encoder->timestamp - last_timestamp;
    dt /= 60000000.0;

    double P = 10.0;
    double I = 0.1;

    double ticks = encoder->left;
    ticks /= 1024.0; // number of rotations since last poll

    double rpm = ticks / dt;
    double error = (rpm - setpoint);

    // update the running integral
    left_integral += (error * dt);

    double output = (P * error) + (I * left_integral);

    cout << "Measured speed: " << rpm << ", PI(D) output: " << output << endl;
    last_timestamp = encoder->timestamp;
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

    drive_train = new DriveTrain(argv[1]);

    // start the asynch loop
    auto loop = CPJL_Message::loop();

    // sleep forever
    while(true)
        usleep(1000000);

    return 0;
}