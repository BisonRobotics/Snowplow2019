#include <iostream>
#include <CPJL.hpp>
#include <cpp/ImuData.h>

#include <vn/sensors.h>
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

    // setup vecnav communication
    VnSensor vecnav;
    vecnav.connect(mount_loc, 115200);
    auto mn = vecnav.readModelNumber();
    cout << "Model number: " << mn << endl;

    auto imu = new ImuData(new CPJL("localhost", 14000), topicname);

    while(true) {
        // read data
        auto reg = vecnav.measure_everything();

        // package data
        imu->timestamp = 0L;
        imu->x_acc = reg.accel.x;
        imu->y_acc = reg.accel.y;

        // send data
        imu->putMessage();
        usleep(1000000); // ~1Hz

    }

    return 0;
}