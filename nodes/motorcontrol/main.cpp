#include <iostream>
#include <mutex>
#include <fstream>

// CPJL interface layer and 
// message type
#include <CPJL.hpp>
#include <cpp/XboxData.h>
#include <cpp/Encoder.h>

// interface library for RoboteQ 
// motor controller
#include <DriveTrain.h>

#define LEFT_MOTOR_CMD(amt) drive_train->wheelVelocity(-amt, RoboteqChannel_1, true, true)
#define RIGHT_MOTOR_CMD(amt) drive_train->wheelVelocity(amt, RoboteqChannel_2, true, true)

#include <misc.h>
#include <unistd.h>

using namespace std;

Encoder* encoder = NULL;
XboxData* xbox_data_rx = NULL;
DriveTrain* drive_train = NULL;

int clamp_motor(int value) {
    if(value > 1000)
        return 1000;
    else if(value < -1000)
        return -1000;
    return value;
}

uint64_t last_timestamp = -1;

double left_setpoint = 0.0;  // unit: RPM
double right_setpoint = 0.0; // ...

int left_output_command = 0;  // -1000 -> +1000
int right_output_command = 0; // ...
const int adjust_amount = 10; // higher value will react faster but may oscillate too much

// loop is cancelled by locking this
mutex loop_mtx;

void encoder_callback(void) {
    double dt = encoder->timestamp - last_timestamp;
    dt /= (60.0 * 1000000.0); // microseconds / minute

    // left wheel
    double left_ticks = encoder->left;
    left_ticks /= 2048.0; // number of rotations since last poll
    double left_rpm = left_ticks / dt;
    double tmp_left_setpoint = left_setpoint;
    if(left_rpm < tmp_left_setpoint)
        left_output_command += adjust_amount;
    else if(left_rpm > tmp_left_setpoint)
        left_output_command -= adjust_amount;

    // right wheel
    double right_ticks = encoder->right;
    right_ticks /= 2048.0;
    double right_rpm = right_ticks / dt;
    double tmp_right_setpoint = right_setpoint;
    if(right_rpm < tmp_right_setpoint)
        right_output_command -= adjust_amount;
    else if(right_rpm > tmp_right_setpoint)
        right_output_command += adjust_amount;

    left_output_command  = clamp_motor(left_output_command);
    right_output_command = clamp_motor(right_output_command);

    cout << "Measured: L " << left_rpm << ", R " << right_rpm << endl;
    cout << "Setpoint: L " << tmp_left_setpoint << ", R " << tmp_right_setpoint << endl;
    cout << "Output:   L " << left_output_command << ", R " << right_output_command << endl << endl;

    loop_mtx.lock();
    LEFT_MOTOR_CMD(left_output_command);
    RIGHT_MOTOR_CMD(right_output_command);
    loop_mtx.unlock();

    last_timestamp = encoder->timestamp;
}

void callback(void) {
    int left_motor = xbox_data_rx->y_joystick_left;
    int right_motor = xbox_data_rx->y_joystick_right;

    //cout << right_motor << endl;

    left_motor  = (int)mapFloat(left_motor, -32768, 32767, -1000, 1000);
    right_motor = (int)mapFloat(right_motor, -32768, 32767, -1000, 1000);

    left_setpoint = mapFloat(left_motor,  -1000, 1000, 50, -50);
    right_setpoint = mapFloat(right_motor, -1000, 1000, 50, -50);

    if(xbox_data_rx->button_b) {
        loop_mtx.lock();
        for(int i : {0, 1, 2})
            drive_train->wheelHalt(true, true);
        exit(EXIT_SUCCESS);
    }
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
    //drive_train->setWheelVelocityDeadZone(-100, 100);
    drive_train->setWatchdogTimer(0, true, true);

    // start the asynch loop
    auto loop = CPJL_Message::loop();

    // sleep forever
    while(true) usleep(1000000);

    return 0;
}
