#include <iostream>
#include <stdio.h>
#include <math.h>
#include <CPJL.hpp>
#include <cpp/ImuData.h>

#include <vn/ezasyncdata.h>
#include <vn/thread.h>

#include "main.h"


#define CALIBRATION_IMU_TO_ROBO_AZIMUTH     (0)
#define CALIBRATION_IMU_TO_ROBO_ZENITH      (0)
#define PI                                  (3.14159265)
#define DEG_TO_RAD(x)                       (x * (PI/180))
#define RAD_TO_DEG(x)                       (x * (180/PI))


using namespace std;
using namespace vn::math;
using namespace vn::sensors;
using namespace vn::protocol::uart;
using namespace vn::xplat;

std::string topicname = "vecnavdata";

void imu_vector_to_robo_vector(float &x, float &y, float &z)
{
    // Convert to sperical coordinates
    float r = sqrt((x*x) + (y*y) + (z*z));
    float azimuth = atanf(z/sqrt(x*x + y*y)); // the skewered egg
    float zenith = atanf(y/x); // theta

    // Apply transformation with values obtained from calibration
    azimuth -= CALIBRATION_IMU_TO_ROBO_AZIMUTH;
    zenith += CALIBRATION_IMU_TO_ROBO_ZENITH;

    // Convert back to cartesian
    x = r * sin(zenith) * cos(azimuth);
    y = r * sin(zenith) * sin(azimuth);
    z = r * cos(zenith);
}

int main(int argc, char* argv[]) {

    std::string mount_loc;
    if(argc != 2) {
        cout << "Usage: " << argv[0] << " <vectornav mount location>" << endl;
        return 1;
    }
    mount_loc = argv[1];

    // prepare to send IMU data
    auto imu = new ImuData(new CPJL("localhost", 14000), topicname);

    // prepare IMU
    auto* ez = EzAsyncData::connect(mount_loc, 115200);

    long int timestamp_val = 0L;
    while(true) {
        imu->timestamp = (++timestamp_val);

        // grab all data from VecNav
        auto cd = ez->currentData();

        // fill out appropriate message fields if data is
        // available. if no new data is ready, send the
        // previous value
        float tempx, tempy, tempz;
        if(cd.hasYawPitchRoll()) {
            auto ypr = cd.yawPitchRoll();
            tempx = ypr.z;
            tempy = ypr.y;
            tempz = ypr.x;

            imu->z_orient = tempz;
        }
        if(cd.hasAcceleration()) {
            auto accel = cd.acceleration();
            tempx = accel.x;
            tempy = accel.y;
            tempz = accel.z;
            imu_vector_to_robo_vector(tempx,tempy,tempz);
            imu->x_acc = tempx;
            imu->y_acc = tempy;
        }
        if(cd.hasAnyVelocity()) {
            auto vel = cd.anyVelocity();
            tempx = vel.x;
            tempy = vel.y;
            tempz = vel.z;
            imu_vector_to_robo_vector(tempx,tempy,tempz);
            imu->x_vel = tempx;
            imu->y_vel = tempy;
        }
        printf("Z orient: %2.2f    x vel: %2.2f    x accel: %2.2f    y vel: %2.2f    y accel: %2.2f", imu->z_orient, imu->x_vel, imu->x_acc, imu->y_vel, imu->y_acc);

        // send data
        imu->putMessage();
        usleep(10000); // ~100Hz
    }

    return 0;
}