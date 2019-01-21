#include <iostream>
#include <CPJL.hpp>
#include <cpp/ImuData.h>

#include <vn/ezasyncdata.h>
#include <vn/thread.h>

#include "main.h"

using namespace std;
using namespace vn::math;
using namespace vn::sensors;
using namespace vn::protocol::uart;
using namespace vn::xplat;

std::string topicname = "vecnavdata";

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
        if(cd.hasYawPitchRoll()) {
            auto ypr = cd.yawPitchRoll();
            imu->z_orient = ypr.z;
        }
        if(cd.hasAcceleration()) {
            auto accel = cd.acceleration();
            imu->x_acc = accel.x;
            imu->y_acc = accel.y;
        }
        if(cd.hasAnyVelocity()) {
            auto vel = cd.anyVelocity();
            imu->x_vel = vel.x;
            imu->y_vel = vel.y;
        }

        // send data
        imu->putMessage();
        usleep(10000); // ~100Hz
    }

    return 0;
}