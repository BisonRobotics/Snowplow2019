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

using namespace std;

XboxData* xbox_data_rx = NULL;
MotorControlCommand* motor_control_command = NULL;
ImuData* imu_data_rx = NULL;
PathVector* AutoVector = NULL;
PathVector* pathStatus = NULL;
Encoder* encoder = NULL;

uint64_t last_timestamp = -1;

#define ENCODER_TICKS_PER_ROTATION      (2048.0)
#define WHEEL_CIRCUMFERENCE             (1.39) // meters

float current_x_acc;
float current_y_acc;
float current_x_vel;
float current_y_vel;
float current_z_orient;
float last_z_orient;
float distanceTraveled_meters;


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
}

/**
 * The Auto Callback converts current location and pathvector to motorspeeds
 */
void auto_callback(void){

    //get target info
    cout << "received new message: Mag: "<< AutoVector->mag << " Dir: "<< AutoVector->dir<< endl;
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

    if(abs(distanceTraveled_meters - targetDirection) <= .1 ){
        //request new vector. we reached destination
        pathStatus->dir = diff;
        pathStatus->mag = distanceTraveled_meters;
        pathStatus->status = "ReachedTarget;SendNewVector";
        pathStatus->putMessage();
        cout<< "requesting New Vector" << endl;
    }else{
        // otherwise drive to vector
        motor_control_command->left = left_motor_speed;
        motor_control_command->right = right_motor_speed;
        motor_control_command->putMessage();
    }
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
        //the Response message node
        pathStatus = new PathVector( new CPJL( "localhost", 14000),
                                    "path_status");
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
