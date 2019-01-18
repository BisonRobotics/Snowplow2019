#include <iostream>
#include <unistd.h>

// CPJL libs and message types
#include <CPJL.hpp>
#include <cpp/SickMeasurement.h>
#include <misc.h>

// SICK interface class. easy
// login and data collection
#include <SICK_Sensor.h>

// comment out to use the actual SICK
// device. otherwise, just packages fake data in a loop
//#define TEST_MODE

using namespace std;

SickMeasurement* sick_measure = NULL;

int main(int argc, char* argv[]) {
    // node that transmits SICK data
    sick_measure = new SickMeasurement(new CPJL("localhost", 14000), "sick_data");

#ifdef TEST_MODE
    int current_value = 100;
    while(true) {
        // cap data at 2500 (2.5m)
        if(current_value > 2500)
            current_value = 100;

        // package data
        for(int i = 0; i < 541; i++)
            sick_measure->data[i] = (float)current_value;
        sick_measure->timestamp = UsecTimestamp();
        sick_measure->putMessage();

        current_value += 10;

        usleep(25000); // ~40Hz
    }

#else // !TEST_MODE

    // data collection endpoint for physical SICK device
    SickSensor sick("192.168.0.1", 2111);

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

#endif // TEST_MODE

    return 0;
}
