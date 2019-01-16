#include <iostream>

// CPJL libs and message types
#include <CPJL.hpp>
#include <cpp/SickMeasurement.h>

// SICK interface class. easy 
// login and data collection
#include <SICK_Sensor.h>

using namespace std;

SickMeasurement* sick_measure = NULL;

int main(int argc, char* argv[]) {
    // node that transmits SICK data
    sick_measure = new SickMeasurement(new CPJL("localhost", 14000), "sick_data");

    // data collection endpoint for physical SICK device
    SickSensor sick;
    
    while(true) {
        // get data from the SICK
        sick.scanData();
        auto& results = sick.getMeasurementResults();

        // pack the data into our messages. this can prolly be replaced by a single system call
        for(int i = 0; i < 541; i++)
            sick_measure->data[i] = results[i];

        // (full) send that bad boy
        sick_measure->putMessage();
    }

    return 0;
}