#include <iostream>
#include <string>

// CPJL interface layer and
// message type
#include <CPJL.hpp>
#include <cpp/XboxData.h>
#include <cpp/Encoder.h>
#include <cpp/MotorControlCommand.h>
#include <cpp/PathVector.h>
#include <cpp/ImuData.h>

#include <misc.h>
#include <unistd.h>

#define ENCODER_TICKS_PER_ROTATION      (2048.0)
#define DEADZONE                        (1)
#define WHEEL_CIRCUMFERENCE             (1.39) // meters


enum wheels_e
{
    LEFT_WHEEL = 0,
    RIGHT_WHEEL,
    NUM_OF_WHEELS
};

using namespace std;

XboxData* xbox_data_rx = NULL;
MotorControlCommand* motor_control_command = NULL;
ImuData* imu_data_rx = NULL;
PathVector* AutoVector = NULL;
Encoder* encoder = NULL;

uint64_t last_timestamp = -1;
float left_setpoint = 0, right_setpoint = 0;

mutex setpoint_mutex;


float current_x_acc;
float current_y_acc;
float current_x_vel;
float current_y_vel;
float current_z_orient;
float last_z_orient;
float distanceTraveled_meters;


int clamp(int value, const int min, const int max) {
    int returnVal = value;
    if(value > max)
        returnVal = max;
    else if(value < min)
        returnVal = min;
    return returnVal;
}

double encoderDataToRPM(double encoderTicks, double timeDelta)
{
    double rotations = encoderTicks / ENCODER_TICKS_PER_ROTATION; // number of rotations since last poll
    double rpm = rotations / timeDelta;
    return rpm;
}

void xbox_callback(void)
{
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

void imu_calback(void)
{
    //receive data from IMU
    current_x_acc = imu_data_rx->x_acc;
    current_y_acc = imu_data_rx->y_acc;
    current_x_vel = imu_data_rx->x_vel;
    current_y_vel = imu_data_rx->y_vel;
    current_z_orient = imu_data_rx->z_orient;

    //TODO: Convert imu data to motor control commands
}

void encoder_callback(void){

    double rotations = encoder->left / ENCODER_TICKS_PER_ROTATION;
    rotations += encoder->right / ENCODER_TICKS_PER_ROTATION;
    rotations /=2; // get the average rotations of the encoders
    distanceTraveled_meters = rotations * WHEEL_CIRCUMFERENCE;

    static uint64_t last_encoder_timestamp;
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

    motor_control_command->left = cmd[LEFT_WHEEL];
    motor_control_command->right = cmd[RIGHT_WHEEL];
    motor_control_command->putMessage();

    for(int i=0 ; i<NUM_OF_WHEELS ; i++)
    {
        prev_cmd[i] = cmd[i];
    }

    last_encoder_timestamp = (uint64_t)encoder->timestamp;
}

/**
 * The Auto Callback converts current location and pathvector to motorspeeds
 */
void auto_callback(void){

    //get target info
    float targetDistance = AutoVector->mag;
    float targetDirection = AutoVector->dir;
    int left_motor_speed = 0; // defualt to full stop
    int right_motor_speed = 0;

    //check if we are facing the right direction
    float diff = current_z_orient - targetDirection;
    if (abs(diff) < 1){
        //drive forward
        left_motor_speed  = 30;
        right_motor_speed = 30;
    }else if (diff < 0 ) {// if dif is negative spin clockwise
        distanceTraveled_meters =0;
        left_motor_speed  = 10;
        right_motor_speed = -10;
    }else if (diff > 0){ // if dif is positive spin counter clockwise
        distanceTraveled_meters =0;
        left_motor_speed  = -10;
        right_motor_speed = 10;
    }

    setpoint_mutex.lock();
    left_setpoint = left_motor_speed;
    left_setpoint = right_motor_speed;
    setpoint_mutex.unlock();
}

int main(int argc, char* argv[])
{
    if(argc != 2) {
        cout << "Usage:\n  " << argv[0] << " <mode>\n";
        exit(EXIT_FAILURE);
    }

    //current mode as the second argument
    string currentMode = argv[1];

    //State machine for the modes' callbacks
    //ignores unwanted data
    if(currentMode == "TELEOP" )
    {
        xbox_data_rx = new XboxData(
                new CPJL("localhost", 14000),
                "xbox_data",
                xbox_callback
            );

    }else if (currentMode == "Auto")
    {
        AutoVector = new PathVector(
                new CPJL("localhost", 14000),
                "path_vector",
                auto_callback);

        imu_data_rx = new ImuData(
                new CPJL("localhost", 14000),
                "imuData",
                imu_calback);

        encoder = new Encoder(
                new CPJL("localhost", 14000),
                "encoder_data",
                encoder_callback);
    }else
    {
        cout << "Incorrect argument:\n" << argv[1] << " <'TELEOP', 'Auto'>\n";
        exit(EXIT_FAILURE);
    }

    motor_control_command = new MotorControlCommand(
        new CPJL("localhost", 14000),
        "motor_control_data"
    );

    // start the asynch loop
    auto loop = CPJL_Message::loop();

    // sleep forever
    while(true)
        usleep(1000000);

    return 0;
}
