#include <iostream>
#include <CPJL.hpp>
#include <cpp/ImuData.h>

using namespace std;

std::string topicname = "vecnavdata";

int main(int argc, char* argv[]) {

    auto imu = new ImuData(new CPJL("localhost", 14000), topicname);

    // main loop
    // 
    // 1.) read data from IMU
    //          - use whatever data structures or methods you wish 
    //            to do so
    //
    // 2.) put data in ImuData message
    //          - all messages have a well-defined structure that is 
    //            the same across all supported languages. you could 
    //            just as easily write this node in Java
    //
    // 3.) send ImuData message out
    //          - this process doesnt care about who might be listening

    while(true) {
        // read data


        // package data


        // send data
        //imu->putMessage();
    }

    return 0;
}