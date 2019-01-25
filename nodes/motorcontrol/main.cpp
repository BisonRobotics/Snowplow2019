#include <iostream>
#include <mutex>
#include <fstream>

// CPJL interface layer and
// message type
#include <CPJL.hpp>
#include <cpp/MotorControlCommand.h>

// interface library for RoboteQ
// motor controller
#include <DriveTrain.h>

#define LEFT_MOTOR_CMD(amt) drive_train->wheelVelocity(-amt, RoboteqChannel_1, true, true)
#define RIGHT_MOTOR_CMD(amt) drive_train->wheelVelocity(-amt, RoboteqChannel_2, true, true)
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

uint64_t last_command_timestamp = -1;
bool watchdog_triggered = false;

const int adjust_amount = 10; // higher value will react faster but may oscillate too much

// loop is cancelled by locking this
mutex loop_mtx;


void command_callback(void) {
    int temp_left_cmd = motor_control_command_rx->left;
    int temp_right_cmd = motor_control_command_rx->right;

    last_command_timestamp = get_us_timestamp();

    if(!watchdog_triggered)
    {
        loop_mtx.lock();
        LEFT_MOTOR_CMD(temp_left_cmd);
        RIGHT_MOTOR_CMD(temp_right_cmd);
        loop_mtx.unlock();
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

    drive_train = new DriveTrain(argv[1]);
    //drive_train->setWheelVelocityDeadZone(-100, 100);
    drive_train->setWatchdogTimer(0, true, true);

    // start the asynch loop
    auto loop = CPJL_Message::loop();
    (void)loop;

    // monitor for broken communication link
    while(true)
    {
        // uint64_t curTime = get_us_timestamp();
        // if((curTime - last_command_timestamp) < COMMAND_TIMEOUT_US)
        // {
        //     // No watchdog trigger
        //     watchdog_triggered = false;
        //     usleep((last_command_timestamp + COMMAND_TIMEOUT_US) - curTime);
        // }
        // else
        // {
        //     // Watchdog trigger
        //     watchdog_triggered = true;

        //     loop_mtx.lock();
        //     LEFT_MOTOR_CMD(0);
        //     RIGHT_MOTOR_CMD(0);
        //     loop_mtx.unlock();

            //last_command_timestamp = curTime + COMMAND_TIMEOUT_US; // push out watchdog time to avoid a loop without sleep
            usleep(500000);
        // }
    }

    return 0;
}
