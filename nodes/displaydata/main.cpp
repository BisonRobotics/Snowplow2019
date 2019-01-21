#include <iostream>
#include <unistd.h>

// CPJL interface layer and 
// message types
#include <CPJL.hpp>
#include <cpp/XboxData.h>
#include <cpp/Encoder.h>
#include <cpp/ImuData.h>
#include <cpp/PathVector.h>

// utility lib, misc. functions
#include <misc.h>
#include <NcursesUtility.h>

using namespace std;

Encoder* encoder_data_rx = NULL;
XboxData* xbox_data_rx = NULL;
PathVector* path_vector = NULL;
ImuData* imu_data_rx = NULL;

int left_motor_raw;
int right_motor_raw;
int left_motor_cmd;
int right_motor_cmd;

float path_vector_mag;
float path_vector_dir;

int left_encoder;
int right_encoder;

float imu_x_acc;
float imu_y_acc;
float imu_x_vel;
float imu_y_vel;
float imu_z_orient;

long int timestamp_encoder;
long int timestamp_pathvector;
long int timestamp_imudata;

void encoder_callback(void) 
{
    left_encoder = encoder_data_rx->left;
    right_encoder = encoder_data_rx->right;
    timestamp_encoder = encoder_data_rx->timestamp;

    //send data to display
}

void xbox_callback(void) 
{
    left_motor_raw = xbox_data_rx->y_joystick_left;
    right_motor_raw = xbox_data_rx->y_joystick_right;

    left_motor_cmd = (int)mapFloat(left_motor_raw, -32768, 32767, -50, 50);
    right_motor_cmd = (int)mapFloat(right_motor_raw, -32768, 32767, -50, 50);

}

void pathvector_callback(void)
{
    path_vector_mag = path_vector->mag;
    path_vector_dir = path_vector->dir;

    timestamp_pathvector = path_vector->timestamp;
}

void imu_callback(void)
{
    imu_x_acc = imu_data_rx->x_acc;
    imu_y_acc = imu_data_rx->y_acc;
    imu_x_vel = imu_data_rx->x_vel;
    imu_y_vel = imu_data_rx->y_vel;
    imu_z_orient = imu_data_rx->z_orient;
}

int main(int argc, char* argv[]) {

    if(argc != 2) {
        cout << "Usage:\n  " << argv[0] << " <roboteq mount location>\n";
        exit(EXIT_FAILURE);
    }

    encoder_data_rx = new Encoder(
        new CPJL("localhost", 14000),
        "encoder_data",
        encoder_callback
    );

    xbox_data_rx = new XboxData(
        new CPJL("localhost", 14000), 
        "xbox_data",
        xbox_callback
    );

    path_vector = new PathVector(
            new CPJL("localhost", 14000),
            "pathvector_data",
            pathvector_callback
    );

    imu_data_rx = new ImuData(
        new CPJL("localhost", 14000),
        "imu_data",
        imu_callback    
    );



    // start the asynch loop
    auto loop = CPJL_Message::loop();

    // sleep forever
    while(true)
        usleep(1000000);

    return 0;
}