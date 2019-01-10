#include <iostream>

#include <CPJL.hpp>
#include <cpp/XboxData.h>

// the xbox interface library
#include <XboxControllerInterface.h>

#include <unistd.h>

using namespace std;

XboxData* xbox_data_tx = NULL;

int main(int argc, char* argv[]) {

    if(argc != 2) {
        cout << "Usage\n  " << argv[0] 
        << " <address of CPJL server>\n";
        return 1;
    }

    xbox_data_tx = 
        new XboxData(
            new CPJL(argv[1], 14000), "xbox_data");

    XboxController xbox;

    while(true) {
        xbox.update();

        xbox_data_tx->x_joystick_left 
            = xbox.getJoyX(xBox_STICK::xBox_LEFT);

        xbox_data_tx->y_joystick_left
            = xbox.getJoyY(xBox_STICK::xBox_LEFT);

        xbox_data_tx->x_joystick_right
            = xbox.getJoyX(xBox_STICK::xBox_RIGHT);

        xbox_data_tx->y_joystick_right
            = xbox.getJoyY(xBox_STICK::xBox_RIGHT);

        xbox_data_tx->button_a
            = xbox.buttonPressed(xBox_A);

        xbox_data_tx->button_b
            = xbox.buttonPressed(xBox_B);

        xbox_data_tx->putMessage();

        usleep(100000);
    }

    return 0;
}