#include <serial-interface.h>

SC::SerialController(void) {
    ;
}

SC::SerialController(const char* serialPort) {
    set_SerialPort(serialPort);
}

int SC::updateBuffer(std::vector<char>& vec) {
    char buf[1024];
    int i = read(fd, buf, 1024);

    if(i > 0) {
        std::cout << "DATA RECEIVED!!\n";
        for(int j = 0; j < i; j++)
            vec.push_back(buf[j]);
    }

    return i;
}

void SC::set_SerialPort(const char* serialPort) {
    memset(&tty, 0, sizeof tty);
    //fd = open(serialPort, O_RDWR | O_NOCTTY | O_NDELAY); // pretty standard flags
    fd = open(serialPort, O_RDWR | O_NOCTTY);
    if(fd < 0) {
        int e = errno;
        std::cerr << "Error opening file" << std::endl;
        std::cerr << "    Error code: " << e << std::endl;
        std::cerr << " -- " << strerror(e) << std::endl;
        exit(-1);
    }

    if(tcgetattr(fd, &tty) < 0) {
        int e = errno;
        std::cerr << "Error retrieving attributes" << std::endl;
        std::cerr << "    Error code: " << e << std::endl;
        std::cerr << " -- " << strerror(e) << std::endl;
        exit(-1);
    }

    serialPortSet = true;
}

void SC::parityEnable(void) {
    tty.c_cflag |= PARENB;
}

int SC::writeBuffer(char* buffer, int bufSize) {
    return write(fd, buffer, bufSize);
}

int SC::readBuffer(char* buffer, int bufSize) {
    return read(fd, buffer, bufSize);
}

void SC::readChunk(char* buffer, int bufSize) {
    int bytes_read = 0;
    while(bytes_read < bufSize)
        bytes_read += read(fd, buffer+bytes_read, bufSize-bytes_read);
}

void SC::writeChunk(char* buffer, int bufSize) {
    int bytes_writ = 0;
    while(bytes_writ < bufSize)
        bytes_writ += write(fd, buffer+bytes_writ, bufSize-bytes_writ);
}

void SC::set_BaudRate(BaudRate baudrate) {
    // most systems do not support different input and output speeds
    // so we set both to the same value
    cfsetispeed(&tty, (int)baudrate); // input speed
    cfsetospeed(&tty, (int)baudrate); // output speed
}

void SC::set_Parity(Parity parity) {
    switch(parity) {
        case Parity_EVEN:
            tty.c_cflag |= PARENB;
            tty.c_cflag &= ~PARODD;
            break;
        case Parity_ODD:
            tty.c_cflag |= PARENB;
            tty.c_cflag |= PARODD;
            break;
        case Parity_NONE: // disable the parity bit
            tty.c_cflag &= ~PARENB;
            break;
        default:
            std::cerr << "Invalid parity argument" << std::endl;
            exit(-1); // all error comditions return -1
    }
}

void SC::set_StopBits(StopBits stopbits) {
    switch(stopbits) {
        case StopBits_1:
            tty.c_cflag &= ~CSTOPB;
            break;
        case StopBits_2:
            tty.c_cflag |= CSTOPB;
            break;
        default:
            std::cerr << "Invalid stop bit argument" << std::endl;
            exit(-1);
    }
}

void SC::set_WordSize(WordSize wordsize) {
    switch(wordsize) {
        case WordSize_5:
            tty.c_cflag &= ~CSIZE;
            tty.c_cflag |= CS5;
            break;
        case WordSize_6:
            tty.c_cflag &= ~CSIZE;
            tty.c_cflag |= CS6;
            break;
        case WordSize_7:
            tty.c_cflag &= ~CSIZE;
            tty.c_cflag |= CS7;
            break;
        case WordSize_8:
            tty.c_cflag &= ~CSIZE;
            tty.c_cflag |= CS8;
            break;
        default:
            std::cerr << "Invalid word size argument" << std::endl;
            exit(-1);
    }
}

void SerialController::start(void) {
    if(serialPortSet == false) {
        std::cerr << "Serial port has not been opened" << std::endl;
        exit(-1);
    }

    if(tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "Error starting serial communications" << std::endl;
        std::cerr << "    Error code: " << errno << std::endl;
        exit(-1);
    }
}

int SerialController::get_FileDescriptor(void) {
    return fd;
}

