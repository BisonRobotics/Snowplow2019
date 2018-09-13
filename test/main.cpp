#include <iostream>
#include <unistd.h>

// VDC2450 motor controller
#include <DriveTrain.h>

using namespace std;

int main(int argc, char* argv[]) {
    
    if(argc != 2) {
        cout << "Supply /dev/<src> as motor controller\n";
        return -1;
    }

    DriveTrain rqi(argv[1]);

    for(;;) {
        rqi.wheelVelocity(1000, RoboteqChannel_2);
        usleep(100000);
    }

    return 0;
}