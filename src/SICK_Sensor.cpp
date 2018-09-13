#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <algorithm>

#include <SICK_Sensor.h>

#define __DEBUG_SCANDATA__

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
    // Thank you Alan Turing
    const int STATE_default     = 0;
    const int STATE_space       = 1;
    const int STATE_token_start = 2;
    const int STATE_token_body  = 3;

    int current_state = STATE_default;

    for(int i = 0; i < _reply_buffer.size();) {

        switch(current_state) {
            case STATE_default:
                if(_reply_buffer[i] == ' ') {
                    current_state = STATE_space;
                } else {
                    current_state = STATE_token_start;
                }
                break;

            case STATE_space:
                // advance until next char
                if(_reply_buffer[i] == ' ') {
                    _reply_buffer[i] = '\0';
                    i++;
                } else {
                    current_state = STATE_token_start;
                }
                break;

            case STATE_token_start:
                _offset_buffer.push_back(i); // save index of this token
                current_state = STATE_token_body;
                i++;
                break;
                
            case STATE_token_body:
                if(_reply_buffer[i] == ' ') {
                    current_state = STATE_space;
                } else {
                    i++; // advance until next space appears
                }
                break;

            default:
                return false;
        }

    }

    return true;
}

bool SickSensor::scanData(void) {
    this->sendCmd("sRN LMDscandata");
    this->readReply();

    // need one more space for token splitting method
    this->_reply_buffer.push_back(' ');    
    this->splitMessageData();

    std::vector<char>& buffer    = this->_reply_buffer;
    std::vector<int>& buf_offset = this->_offset_buffer;

    // some debug info if needed
    #ifdef __DEBUG_SCANDATA__
    for(int i : buf_offset)
        std::cout << &buffer[i] << std::endl;
    #endif

    // we havent thrown an error by now, find the tokens we need
    int data_index = -1;
    for(int i = 0; i < buf_offset.size(); i++) {
        if(strcmp(&buffer[buf_offset[i]], "DIST1") == 0) {
            data_index = i;
            break;
        }
    }

    if(data_index < 0)
        return false; // no DIST1 token, quit

    return true;
}

// this method is only used for testin purposes
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
    this->_meas_results.clear();

    close(fd); // we no longer need the file

    for(int i = 0; i < this->_offset_buffer.size(); i++) {

        int index = _offset_buffer[i];

        if(strcmp(&_reply_buffer[index], "DIST1") == 0) {
            std::cout << "DIST1 index: " << index << std::endl;

            // print the next few ints
            for(int j = 0; j < 20; j++)
                std::cout << hexStrToInt(&this->_reply_buffer[_offset_buffer[i+j+1]]) << "    "
                << std::stol(&_reply_buffer[_offset_buffer[i+j+1]], 0, 16) << std::endl;
            return true;
        }
    }

    return true;
}

void SickSensor::sendCmd(std::string cmd) {
    char stx = 0x02;
    char etx = 0x03;

    // start transmission
    this->tc.writeSocket(&stx, 1);

    // the actual command
    this->tc.writeSocket((char*)cmd.c_str(), cmd.size());

    // end transmission
    this->tc.writeSocket(&etx, 1);
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
        char c_buf[64];
        int n = tc.readSocket(c_buf, 64);

        for(int i = 0; i < n; i++) {
            if(c_buf[i] == 0x03)
                return true;
            this->_reply_buffer.push_back(c_buf[i]); // fill the buffer until ETX is found
        }

        // max size of the return buffer
        if(_reply_buffer.size() > 10000)
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

    std::vector<char> vec_str;
    for(int i = 0; str[i]; i++)
        vec_str.push_back(str[i]);
    std::reverse(vec_str.begin(), vec_str.end());
    vec_str.push_back('\0');

    // build up the hex number 4 bits at a time
    int64_t r = 0L;

    for(int index = 0; vec_str[index]; index++) {
        int64_t tmp = hexLookup(vec_str[index]) & 15L;
        tmp <<= (index * 4);
        r |= tmp;
    }

    return r;

}

// welcome to the Mystery Machine
