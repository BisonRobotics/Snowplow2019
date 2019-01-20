#include <iostream>
#include <NcursesUtility.h>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {

    NcursesUtility nu;

    auto solid_red =           nu.initializeSolidColor(COLOR_RED);
    auto box_color =           nu.initializeSolidColor(COLOR_WHITE);
    auto primary_lettering =   nu.initializeColor(COLOR_BLUE, COLOR_BLACK);
    auto secondary_lettering = nu.initializeColor(COLOR_CYAN, COLOR_BLACK);

    int iter = 0;

    bool looping = true;
    while(looping) {
        nu.clear();

        nu.displayStringAt("IMU: ", 1, 2, primary_lettering);
        nu.displayStringAt("GPS: ", 2, 2, primary_lettering);
        nu.displayStringAt(to_string(iter), 2, 7, secondary_lettering);
        nu.drawHorizontalLine(3, solid_red);
        nu.displayStringAt("Press ESC to exit...", 4, 2, primary_lettering);

        // draw a box around the important bits
        nu.drawBox({0, 0, 6, 24}, box_color);

        nu.flip();
        
        // ~10Hz
        if(nu.getCharacter(100) == 27)
            looping = false;

        iter++;
    }

    return 0;
}
