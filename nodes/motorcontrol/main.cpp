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

#include <misc.h>
#include <unistd.h>

using namespace std;

Encoder* encoder = NULL;
XboxData* xbox_data_rx = NULL;
DriveTrain* drive_train = NULL;

uint64_t last_timestamp = -1;

double left_pre_error = 0.0;
double right_pre_error = 0.0;

double left_integral = 0.0;
double right_integral = 0.0;

// PID gains
double P = 0.0; // Proportional
double I = 0.0; // Integral
double D = 0.0; // Derivative

mutex setpt_mtx;
double setpoint = 50.0; // unit: RPM

// loop is cancelled by locking this
mutex loop_mtx;

void encoder_callback(void) {
    double dt = encoder->timestamp - last_timestamp;
    dt /= (60.0 * 1000000.0); // microseconds/minute

    double ticks = encoder->left;
    ticks /= 2048.0; // number of rotations since last poll

    double rpm = ticks / dt;

    setpt_mtx.lock();
    double error = (rpm - setpoint);
    setpt_mtx.unlock();

    // update the running integral
    left_integral += (error * dt);
    double left_derivative = (error - left_pre_error) / dt;

    double output = (P * error) + (I * left_integral) + (D * left_derivative);

    cout << "Measured speed: " << rpm << ", Setpoint: " << setpoint << ", Error: " << error << ", PID output: " << output << endl;

    if(output > 1000)
        output = 1000;
    else if(output < -1000)
        output = -1000;

    loop_mtx.lock();
    drive_train->wheelVelocity((int)output, RoboteqChannel_1, true, true);
    loop_mtx.unlock();

    last_timestamp = encoder->timestamp;
    left_pre_error = error;
}

void callback(void) {
    int left_motor = xbox_data_rx->y_joystick_left;
    int right_motor = xbox_data_rx->y_joystick_right;

    left_motor  = (int)mapFloat(left_motor, -32768, 32767, -1000, 1000);
    right_motor = (int)mapFloat(right_motor, -32768, 32767, -1000, 1000);

    setpt_mtx.lock();
    setpoint = mapFloat(left_motor,  -1000, 1000, 50, -50);
    setpt_mtx.unlock();

    //drive_train->wheelVelocity(right_motor, RoboteqChannel_2, true, true);
    //drive_train->wheelVelocity(left_motor, RoboteqChannel_1, true, true);

    if(xbox_data_rx->button_b) {
        loop_mtx.lock();
        for(int i : {0, 1, 2})
            drive_train->wheelHalt(true, true);
        exit(EXIT_SUCCESS);
    }

    //cout << "Left: " << left_motor << ", Right: " << right_motor << endl;
}

int main(int argc, char* argv[]) {

    if(argc != 2) {
        cout << "Usage:\n  " << argv[0] << " <roboteq mount location>\n";
        exit(EXIT_FAILURE);
    }

    // grab the PID terms from the text file
    {
        ifstream ist("./pid-terms.txt");
        ist >> P >> I >> D;
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
    drive_train->setWheelVelocityDeadZone(-100, 100);
    drive_train->setWatchdogTimer(0, true, true);

    // start the asynch loop
    auto loop = CPJL_Message::loop();

    // sleep forever
    while(true)
        usleep(1000000);

    return 0;
}