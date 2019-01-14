#include <iostream>

// both of these are found in the CPJL repo
#include <CPJL.hpp>
#include <cpp/ImuData.h>

using namespace std;

ImuData* imu = NULL;
std::string topicname = "vecnavdata";

int main(int argc, char* argv[]) {

    imu = new ImuData(new CPJL("localhost", 14000), topicname, 
            [](void) -> void {
                cout << endl
                << "Imu data"
                << "    Time:  " << imu->timestamp << endl
                << "    X Acc: " << imu->x_acc << endl
                << "    Y Acc: " << imu->y_acc << endl
                << "    X Vel: " << imu->x_vel << endl
                << "    Y Vel: " << imu->y_vel << endl;
            });

    // spend forever waiting
    auto loop = CPJL_Message::loop();
    while(true) sleep(1000000);

    return 0;
}