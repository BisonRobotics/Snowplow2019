#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <algorithm>
#include <stdint.h>
#include <sstream>

#include <SICK_Sensor.h>

//#define __DEBUG_SCANDATA__

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

SickSensor::SickSensor(std::string IP, int port_n, bool init) {
    if(!init)
        return; // for testing purposes

    this->tc.set_Hostname(IP.c_str());
    this->tc.set_PortNumber(port_n);
    this->tc.start();
}

void SickSensor::setAccessMode(User user) {
    switch(user) {
        case User::MAINTENANCE:
            this->sendCmd("sMN SetAccessMode 02 B21ACE26");
            break;
        case User::CLIENT:
            this->sendCmd("sMN SetAccessMode 03 F4724744");
            break;
        case User::SERVICE:
            this->sendCmd("sMN SetAccessMode 04 81BE23AA");
            break;
        default:
            throw std::runtime_error(std::string("UNKNOWN TYPE IN SickSensor::setAccessMode"));
    }
}

bool SickSensor::splitMessageData(void) {
    std::stringstream ss;
    ss.write(&this->_reply_buffer[0], this->_reply_buffer.size());

    this->_raw_string_buffer.clear();

    std::string str;
    while(ss >> str) {
        this->_raw_string_buffer.push_back(str);
    }

    return false;
}

bool SickSensor::scanData(void) {
    this->sendCmd("sRN LMDscandata");
    this->_reply_buffer.clear();
    this->readReply();

    this->splitMessageData();

    if(this->scanDataToFloat() == false)
        return false;

    return true;
}

// this method is only used for testing purposes
bool SickSensor::scanData(const char* filename) {
    int fd = open(filename, O_RDONLY);

    if(fd == -1) {
        std::cout << "ERROR OPENING FILE: " << strerror(errno) << std::endl;
        return false;
    }

    char buf[1024*4]; // 4kB

    int n = read(fd, buf, 1024*4);
    this->_reply_buffer.clear();
    for(int i = 1; i < (n-1); i++)
        this->_reply_buffer.push_back(buf[i]);
    this->_reply_buffer.push_back(' ');

    this->splitMessageData();

    close(fd); // we no longer need the file

    if(this->scanDataToFloat() == false)
        return false;

    return true;
}

void SickSensor::sendCmd(std::string cmd) {
    const char stx = 0x02;
    const char etx = 0x03;

    static std::vector<char> cmd_string;
    cmd_string.clear();

    cmd_string.push_back(stx);
    for(char c : cmd)
        cmd_string.push_back(c);
    cmd_string.push_back(etx);

    this->tc.writeSocket(&cmd_string[0], cmd_string.size());

    // start transmission
    //this->tc.writeSocket(&stx, 1);
    // the actual command
    //this->tc.writeSocket((char*)cmd.c_str(), cmd.size());
    // end transmission
    //this->tc.writeSocket(&etx, 1);
}

bool SickSensor::readReply(void) {
    char tmp = '\0';
    this->tc.readSocket(&tmp, 1);
    
    if(tmp != 0x02) {
        std::cerr << "STX NOT FOUND...\n";
        return false;
    }

    // start transmission found
    this->_reply_buffer.clear();

    while(1) {
        char c_buf[512];
        int n = tc.readSocket(c_buf, 512);

        for(int i = 0; i < n; i++) {
            if(c_buf[i] == 0x03)
                return true;
            this->_reply_buffer.push_back(c_buf[i]); // fill the buffer until ETX is found
        }

        // max size of the return buffer
        if(_reply_buffer.size() > 10000) // ~10KB, a single response should NEVER be this big
            return false; // too many chars, something went wrong
    }

}

int64_t SickSensor::hexStrToInt(char* str) {
    // lambdas ftw
    auto hexLookup = [](char c) -> int {
        if(c >= '0' && c <= '9')
            return (c - '0'); // digits 0-9
        else if(c >= 'A' && c <= 'F')
            return (c - 'A' + 10); // uppercase A-F
        else if(c >= 'a' && c <= 'f')
            return (c-'a' + 10); // lowercase a-f
        else
            std::cerr << "UNKNOWN CHAR IN SickSensor::hexStrToInt()\n";
            return 0;
    };

    int strL = strlen(str);

    // build up the binary number 4 bits at a time
    int64_t r = 0L;

    for(int index = strL-1; index >= 0; index--) {
        int64_t tmp = hexLookup(str[index]) & 15L;
        tmp <<= ((strL-(index+1)) * 4);
        r |= tmp;
    }

    return r;
}

bool SickSensor::scanDataToFloat(void) {
    int dist1_index = -1;
    int num_readings = -1;

    for(int i = 0; i < this->_offset_buffer.size(); i++) {
        if(strcmp(&_reply_buffer[_offset_buffer[i]], "DIST1") == 0) {
            dist1_index = i;
            num_readings = (int)hexStrToInt(&_reply_buffer[_offset_buffer[i+5]]);
            break;
        }
    }

    if(dist1_index == -1 || num_readings < 0) { // should be found by above loop
        return false;
    }

    // DIST1 found, read the actual data
    for(int i = 0; i < num_readings; i++) {
        float val = (float)hexStrToInt(&_reply_buffer[_offset_buffer[i+dist1_index+5]]);
        _meas_results.push_back(val);
    }

    return true;
}

auto SickSensor::getMeasurementResults(void) -> std::vector<float>& {
    return this->_meas_results;
}

auto SickSensor::getMeasurementResultsAsCartesian(void) -> std::vector<cart_t>& {
    this->_cart_meas_results_.clear();

    for(int i = 0; i < this->_meas_results.size(); i++) {
        float ang = float(i-90) * 0.00872665f; // theres that magic number again
        float x = ang * cos(_meas_results[i]);
        float y = ang * sin(_meas_results[i]);
        this->_cart_meas_results_.push_back({x, y});
    }

    return this->_cart_meas_results_;
}


// welcome to the Mystery Machine
