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

    //Printing to screen
    NcursesUtility nu;

    auto solid_red =           nu.initializeSolidColor(COLOR_RED);
    auto box_color =           nu.initializeSolidColor(COLOR_WHITE);
    auto primary_lettering =   nu.initializeColor(COLOR_BLUE, COLOR_BLACK);
    auto secondary_lettering = nu.initializeColor(COLOR_CYAN, COLOR_BLACK);


    bool looping = true;
    while(looping) {
        nu.clear();

        nu.displayStringAt("Joystick", 1, 2, primary_lettering);
        nu.displayStringAt("Left Motor Raw: " + to_string(left_motor_raw), 2, 2, secondary lettering);
        nu.displayStringAt("Right Motor Raw" + to_string(right_motor_raw), 3, 2, secondary_lettering);
        nu.displayStringAt("Left Motor CMD: " + to_string(left_motor_cmd), 4, 2, secondary lettering);
        nu.displayStringAt("Right Motor CMD" + to_string(right_motor_cmd), 5, 2, secondary_lettering);
        
        nu.displayStringAt("En
        coder", 7, 2, primary_lettering);
        nu.displayStringAt("Left: " + to_string(left_encoder),8,2,secondary_lettering);
        nu.displayStringAt("Right: " + to_string(right_encoder),9,2,secondary_lettering);
        nu.displayStringAt("  Timestamp: " + to_string(timestamp_encoder), 10, 2, primary_lettering);

        nu.displayStringAt("IMU",12,2,primary_lettering);
        nu.displayStringAt("X Accel: " + to_string(imu_x_acc) + " m/s^2",13,2,secondary_lettering);
        nu.displayStringAt("Y Accel: "+ to_string(imu_y_acc) + " m/s^2",14,2,secondary_lettering);
        nu.displayStringAt("X Vel: " + to_string(imu_x_vel) + " m/s", 15,2, secondary_lettering);
        nu.displayStringAt("Y Vel: " + to_string(imu_y_vel) + " m/s",16,2, secondary_lettering);
        nu.displayStringAt("Z Orient: " + to_string(imu_z_orient)+ "degradians",17,2,secondary_lettering);

        nu.displayStringAt("Path Vector",19,2,primary_lettering);
        nu.displayStringAt("Magn: " + to_string(path_vector_mag,20,2,secondary_lettering);
        nu.displayStringAt("Dir: " + to_string(path_vector_dir),21,2,secondary_lettering);



        nu.flip();
        
        // ~10Hz
        if(nu.getCharacter(100) == 27)
            looping = false;

    }

    // start the asynch loop
    auto loop = CPJL_Message::loop();

    // sleep forever
    while(true)
        usleep(1000000);

    return 0;
}