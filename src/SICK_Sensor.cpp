#include <SICK_Sensor.h>
#include <stdexcept>
#include <string>

#define __DEBUG_SCANDATA__

SickSensor::SickSensor(std::string IP, int port_n) {
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

void SickSensor::splitMessageData(void) {
    // thank you Alan Turing
    const int STATE_default       = 0;
    const int STATE_token_start   = 1;
    const int STATE_token         = 2;
    const int STATE_space         = 3;

    int current_state = STATE_default;

    std::vector<char>& buffer = this->_reply_buffer;
    std::vector<int>&  buf_offset = this->_offset_buffer;

    // find individual tokens
    // this was done after a few brews...
    for(int i = 0; i < buffer.size();) { // dont advance automatically
        switch(current_state) {
            case STATE_default:
                if(buffer[i] == ' ') {
                    current_state = STATE_space;
                } else {
                    current_state = STATE_token_start;
                }
                break;

            case STATE_token_start:
                buf_offset.push_back(i); // there is a string here
                i++;
                break;

            case STATE_token: // find the end of the string
                if(buffer[i] == ' ') {
                    current_state = STATE_space;
                else
                    i++;
                break;

            case STATE_space: // replace spaces and advance the char ptr
                buffer[i] = '\0';
                i++;
                current_state = STATE_default; // let the default state deal with the rest
                break;

            default:
                throw std::runtime_error("SickSensor::scanData : UNKNOWN STATE");
        }
    }
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
    this->tc.readBuffer(&tmp, 1);
    
    if(tmp != 0x02) {
        std::cerr << "STX NOT FOUND...\n";
        return false;
    }

    // start transmission found
    this->_reply_buffer.clear();

    while(1) {
        char c_buf[64];
        int n = tc.readBuffer(c_buf, 64);

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

// welcome to the Mystery Machine
