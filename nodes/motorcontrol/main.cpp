#include <iostream>
#include <mutex>
#include <fstream>

// CPJL interface layer and
// message type
#include <CPJL.hpp>
#include <cpp/MotorControlCommand.h>
#include <cpp/Encoder.h>

// interface library for RoboteQ
// motor controller
#include <DriveTrain.h>

#define LEFT_MOTOR_CMD(amt) drive_train->wheelVelocity(-amt, RoboteqChannel_1, true, true)
#define RIGHT_MOTOR_CMD(amt) drive_train->wheelVelocity(-amt, RoboteqChannel_2, true, true)
#define ENCODER_TICKS_PER_ROTATION      (2048.0)
#define DEADZONE                        (1)
#define COMMAND_TIMEOUT_US              (500000)

#include <misc.h>
#include <unistd.h>

using namespace std;

enum wheels_e
{
    LEFT_WHEEL = 0,
    RIGHT_WHEEL,
    NUM_OF_WHEELS
};

Encoder* encoder = NULL;
MotorControlCommand* motor_control_command_rx = NULL;
DriveTrain* drive_train = NULL;

int clamp(int value, const int min, const int max) {
    int returnVal = value;
    if(value > max)
        returnVal = max;
    else if(value < min)
        returnVal = min;
    return returnVal;
}

uint64_t get_us_timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

double encoderDataToRPM(double encoderTicks, double timeDelta)
{
    double rotations = encoderTicks / ENCODER_TICKS_PER_ROTATION; // number of rotations since last poll
    double rpm = rotations / timeDelta;
    return rpm;
}

uint64_t last_encoder_timestamp = -1;
uint64_t last_command_timestamp = -1;
bool watchdog_triggered = false;

double left_setpoint = 0.0;  // unit: RPM
double right_setpoint = 0.0; // ...

const int adjust_amount = 10; // higher value will react faster but may oscillate too much

// loop is cancelled by locking this
mutex setpoint_mutex;
mutex loop_mtx;

void encoder_callback(void) {
    double dt = encoder->timestamp - last_encoder_timestamp;

    dt /= (60.0 * 1000000.0); // microseconds / minute

    double wheel_rpm[NUM_OF_WHEELS];
    double setpoint[NUM_OF_WHEELS];
    static double prev_cmd[NUM_OF_WHEELS];
    double cmd[NUM_OF_WHEELS];


    // Get wheel RPM
    wheel_rpm[LEFT_WHEEL]  = encoderDataToRPM(encoder->left, dt);
    wheel_rpm[RIGHT_WHEEL] = encoderDataToRPM(encoder->right, dt);

    // Make local copy of setpoints
    setpoint_mutex.lock();
    setpoint[LEFT_WHEEL] = left_setpoint;
    setpoint[RIGHT_WHEEL] = right_setpoint;
    setpoint_mutex.unlock();


    for(int i=0; i<NUM_OF_WHEELS; i++)
    {
        if(setpoint[i] > (double)DEADZONE || setpoint[i] < (double)-DEADZONE)
        {
            if(wheel_rpm[i] < setpoint[i])
            {
                double error = setpoint[i] - wheel_rpm[i];
                cmd[i] = prev_cmd[i] + clamp(((error * error /10) + 1), 0, 50);
            }
            else
            {
                double error = wheel_rpm[i] - setpoint[i];
                cmd[i] = prev_cmd[i] - clamp(((error * error /10) + 1), 0, 50);
            }
        }
        else
        {
            cmd[i] = 0;
        }
        cmd[i] = clamp(cmd[i], -1000, 1000);
    }

#ifndef NDEBUG
    cout << "Measured: L " << wheel_rpm[LEFT_WHEEL] << ", R " << wheel_rpm[RIGHT_WHEEL] << endl;
    cout << "Setpoint: L " << setpoint[LEFT_WHEEL] << ", R " << setpoint[RIGHT_WHEEL] << endl;
    cout << "Output:   L " << cmd[LEFT_WHEEL] << ", R " << cmd[RIGHT_WHEEL] << endl << endl;
#endif //NDEBUG

    loop_mtx.lock();
    LEFT_MOTOR_CMD(cmd[LEFT_WHEEL]);
    RIGHT_MOTOR_CMD(cmd[RIGHT_WHEEL]);
    loop_mtx.unlock();

    for(int i=0 ; i<NUM_OF_WHEELS ; i++)
    {
        prev_cmd[i] = cmd[i];
    }

    last_encoder_timestamp = (uint64_t)encoder->timestamp;
}

void command_callback(void) {
    int temp_left_setpoint = motor_control_command_rx->left;
    int temp_right_setpoint = motor_control_command_rx->right;

    last_command_timestamp = get_us_timestamp();

    //cout << right_motor << endl;

    //left_motor  = (int)mapFloat(left_motor, -32768, 32767, -1000, 1000);
    //right_motor = (int)mapFloat(right_motor, -32768, 32767, -1000, 1000);

    if(!watchdog_triggered)
    {
        setpoint_mutex.lock();
        left_setpoint = (double) temp_left_setpoint;
        right_setpoint = (double) temp_right_setpoint;
        setpoint_mutex.unlock();
    }
}

int main(int argc, char* argv[]) {

    if(argc != 2) {
        cout << "Usage:\n  " << argv[0] << " <roboteq mount location>\n";
        exit(EXIT_FAILURE);
    }

    motor_control_command_rx = new MotorControlCommand(
        new CPJL("localhost", 14000),
        "motor_control_data",
        command_callback
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
    (void)loop;

    // monitor for broken communication link
    while(true)
    {
        uint64_t curTime = get_us_timestamp();
        if((curTime - last_command_timestamp) < COMMAND_TIMEOUT_US)
        {
            // No watchdog trigger
            watchdog_triggered = false;
            usleep((last_command_timestamp + COMMAND_TIMEOUT_US) - curTime);
        }
        else
        {
            // Watchdog trigger
            watchdog_triggered = true;

            loop_mtx.lock();
            LEFT_MOTOR_CMD(0);
            RIGHT_MOTOR_CMD(0);
            loop_mtx.unlock();

            //last_command_timestamp = curTime + COMMAND_TIMEOUT_US; // push out watchdog time to avoid a loop without sleep
            usleep(500000);
        }
    }

    return 0;
}
