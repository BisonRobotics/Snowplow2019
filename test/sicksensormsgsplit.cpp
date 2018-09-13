#include <iostream>
#include <fstream>

#include <SICK_Sensor.h>

using namespace std;

int main(int argc, char* argv[]) {
    SickSensor ss("localhost", 13000, false); // dont actually connect to SICK

    ss.scanData("../txt/sick-response.txt");

}