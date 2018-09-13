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
typedef std::vector<cart_t> CartVec;

enum class User : int {
    MAINTENANCE = 0,
    CLIENT = 1,
    SERVICE = 2
};

std::ostream& operator<<(std::ostream& os, User u);

class SickSensor {
private:
    TCP_Controller tc;

    std::vector<float> _meas_results;
    std::vector<char> _reply_buffer;
    std::vector<int>  _offset_buffer;

    // generic send command method used to send ASCII
    // commands to SICK sensor
    // reading reply is done elsewhere
    void sendCmd(std::string cmd);

    // many commands have a reply that should be read
    // user doesnt need to know the exact mechanism
    // by which the reply is parsed
    // this method simply reads until 0x03 is encountered
    bool readReply(void);

    // used by the scanData method to find indices of 
    // different tokens in the response stream
    bool splitMessageData(void);

    // produces a large integer suitable for use in 
    // converting to other ints, float, or doubles. 
    // assumes c-string is in hex format
    int64_t hexStrToInt(char* s);

public:
    // constructor, connection parameters
    // have defaults. unless you have multiple
    // SICK sensors, the defaults are fine
    SickSensor(std::string IP = "192.168.0.1", int port_n = 2111, bool init = true); // default parameters

    // setting different access modes allows you 
    // to set different parameters on the SICK sensor
    void setAccessMode(User user);

    // get the most recent set of measurements from SICK
    // method allocates all necessary space needed to 
    // store results (floats)
    // returns whether method was able to parse out results
    bool scanData(void);

    // FOR TESTING PURPOSES ONLY!!!!!!!
    // opens an existing file and treats it like it came from 
    // the SICK sensor
    bool scanData(const char* filename);

    // retrieve the results from most recent measurement
    // requires some additional input from programmer
    auto getMeasurementResults(void) -> std::vector<float>&;

    // converts the most recent results to cartesian coordinates
    // user needs to provide an existing CartVec object
    auto getMeasurementResultsAsCartesian(CartVec& cv) -> void;

};

#endif // __JJC__SICK__SENSOR__H__