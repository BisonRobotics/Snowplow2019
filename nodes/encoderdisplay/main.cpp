#include <iostream>

// both of these are found in the CPJL repo
#include <CPJL.hpp>
#include <cpp/Encoder.h>

using namespace std;

Encoder* encoder = NULL;
std::string topicname = "encoder_data";

int main(int argc, char* argv[]) {

    encoder = new Encoder(new CPJL("localhost", 14000), topicname,
        [](void) -> void {
            cout << "Left: " << encoder->left
            << ", right: " << encoder->right << endl;
        });

    // spend forever waiting
    auto loop = CPJL_Message::loop();
    while(true) sleep(1000000);

    return 0;
}