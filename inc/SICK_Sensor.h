#ifndef __JJC__SICK__SENSOR__H__
#define __JJC__SICK__SENSOR__H__

#include <string>
#include <vector>
#include <iostream>
#include <TCP_Connection.h>

struct XY {
    float x, y;
} __attribute__((packed));

// convenience typedef simplifies polar coordinates
// turned to cartesian by SickSensor class
typedef XY cart_t; // very similar to std::pair, nicer names for our purposes
typedef std::vector<XY> CartVec;

enum class User : int {
    MAINTENANCE = 0,
    CLIENT = 1,
    SERVICE = 2
};

std::ostream& operator<<(std::ostream& os, User u) {
    switch(u) {
        case User::MAINTENANCE:
            os << "User::MAINTENANCE";
            break;
        case User::CLIENT:
            os << "User::CLIENT";
            break;
        case User::SERVICE:
            os << "User::SERVICE";
            break;
    }
    return os;
}

class SickSensor {
private:
    TCP_Controller tc;

    std::vector<float> _meas_results;

    // generic send command method used to send ASCII
    // commands to SICK sensor
    // reading reply is done elsewhere
    void sendCmd(std::string cmd);

    // many commands have a reply that should be read
    // user doesnt need to know the exact mechanism
    // by which the reply is parsed
    // this method simply reads until 0x03 is encountered
    void readReply(std::vector<char>& buf);

public:
    // constructor, connection parameters
    // have defaults. unless you have multiple
    // SICK sensors, the default are fine
    SickSensor(std::string IP = "192.168.0.1", int port_n = 2111); // default parameters

    // setting different access modes allows you 
    // to set different parameters on the SICK sensor
    void setAccessMode(User user);

    // get the most recent set of measurements from SICK
    // method allocates all necessary space needed to 
    // store results (floats)
    // returns whether method was able to parse out results
    bool scanData(void);

    // retrieve the results from most recent measurement
    // requires some additional input from programmer
    auto getMeasurementResults(void) -> std::vector<float>&;

    // converts the most recent results to cartesian coordinates
    // user needs to provide an existing CartVec object
    auto getMeasurementResultsAsCartesian(CartVec& cv);

};

#endif // __JJC__SICK__SENSOR__H__