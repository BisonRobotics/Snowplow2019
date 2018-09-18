#include <TCP_Connection.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <stdint.h>

TCP_Controller::TCP_Controller(void) {
    ;
}

TCP_Controller::TCP_Controller(const char* hostname, int port) {
    set_Hostname(hostname);
    set_PortNumber(port);
}

void TCP_Controller::set_Hostname(const char* hostname) {
    this->hostname = hostname;
    this->hostnameSet = true;
}

void TCP_Controller::set_PortNumber(int port) {
    this->port = port;
    this->portnumSet = true;
}

int TCP_Controller::readSocket(char* buffer, int bufSize) {
    int n = read(this->sockfd, buffer, bufSize);

    if(n < 0) {
        std::cerr << "Read error" << std::endl;
        std::cerr << "    code: " << errno << std::endl;
    }

    return n;
}

void TCP_Controller::readUntil(uint8_t flag, std::vector<char>& buf) {
    buf.clear();
    uint8_t c = 0x00;

    // this loop will eventually exit
    while(1) {
        if(read(this->sockfd, &c, 1) == 0)
            return;

        if(c == flag)
            return;
        buf.push_back((char)c);
    }
}

int TCP_Controller::writeSocket(std::string buffer) {
    return write(this->sockfd, buffer.c_str(), buffer.size());
}

int TCP_Controller::writeSocket(char* buffer, int bufSize) {
    return write(this->sockfd, buffer, bufSize);
}

void TCP_Controller::start(void) {

    if(portnumSet == false) {
        std::cerr << "ERROR port number not set" << std::endl;
        exit(-1);
    }

    if(hostnameSet == false) {
        std::cerr << "ERROR hostname not set" << std::endl;
        exit(-1);
    }

    this->portno = port;
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        std::cerr << "ERROR opening socket" << std::endl;
        std::cerr << "    code: " << errno << std::endl;
        exit(-1);
    }

    this->server = gethostbyname(hostname);
    if (server == NULL) {
        std::cerr << "ERROR, no such host: " << hostname << std::endl;
        exit(-1);
    }

    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)  this->server->h_addr,
          (char*) &this->serv_addr.sin_addr.s_addr,
                   this->server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR connecting" << std::endl;
        std::cerr << "Hostname: " << hostname << " Port: " << port << std::endl;
        std::cerr << "    error code: " << errno << std::endl;
        exit(-1);
    }
}
















