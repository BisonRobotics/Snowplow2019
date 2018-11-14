#include <iostream>
#include <CPJL.hpp>
#include <cpp/ImuData.h>

using namespace std;

ImuData* imu = NULL;
std::string topicname = "vecnavdata";

int main(int argc, char* argv[]) {

    imu = new ImuData(new CPJL("localhost", 14000), topicname, 
            [](void) -> void {
                cout 
                << "Imu data"
                << "    X Acc: " << imu->x_acc << endl
                << "    Y Acc: " << imu->y_acc << endl
                << "    X Vel: " << imu->x_vel << endl
                << "    Y Vel: " << imu->y_vel << endl
                << "    X Pos: " << imu->x_pos << endl
                << "    Y Pos: " << imu->y_pos << endl;
            });

    // spend forever waiting
    auto loop = CPJL_Message::loop();
    while(true) sleep(1000000);

    return 0;
}