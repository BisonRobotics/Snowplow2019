/**
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#ifndef __JJC__SERIALCONTROLLER__H__
#define __JJC__SERIALCONTROLLER__H__

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <stdlib.h>

#define SC SerialController

enum Parity {
    Parity_ODD,
    Parity_EVEN,
    Parity_NONE
};

enum WordSize {
    WordSize_5,
    WordSize_6,
    WordSize_7, 
    WordSize_8
};

enum StopBits {
    StopBits_1, 
    StopBits_2
};

enum BaudRate {
    BaudRate_50      = B50,
    BaudRate_75      = B75,
    BaudRate_110     = B110,
    BaudRate_134     = B134,
    BaudRate_150     = B150,
    BaudRate_200     = B200,
    BaudRate_300     = B300,
    BaudRate_600     = B600,
    BaudRate_1200    = B1200,
    BaudRate_2400    = B2400,
    BaudRate_4800    = B4800,
    BaudRate_9600    = B9600,
    BaudRate_19200   = B19200,
    BaudRate_38400   = B38400,
    BaudRate_57600   = B57600,
    BaudRate_115200  = B115200
};

class SerialController {
private:
    termios tty;
    int fd = -1;
    bool serialPortSet = false;
    void parityEnable(void);

public:
    // constructor that does nothing
    SerialController(void);

    // constructor with serial port as argument
    SerialController(const char* serialPort);

    // specify a serial device to use
    void set_SerialPort(const char* serialPort);

    // try to read a certain number of bytes from the serial port
    int readBuffer(char* buffer, int bufSize);

    // try write a certain number of bytes from the serial port
    int writeBuffer(char* buffer, int bufSize);

    // block until operations are complete
    void readChunk(char* buffer, int bufSize);

    // guarantee write bufSize bytes
    void writeChunk(char* buffer, int bufSize);

    // set the read/write speeds for the serial port
    void set_BaudRate(BaudRate baudrate);

    // set odd/even/no parity for this serial port
    void set_Parity(Parity parity);

    // set either 7 or 8 bit word for this serial port
    void set_WordSize(WordSize wordsize);

    // set either 1 or 2 stop bits for this port
    void set_StopBits(StopBits stopbits);

    // start serial communications
    void start(void);

    // return the file descriptor associated with this port
    int get_FileDescriptor(void);

    // update external buffer of data
    int updateBuffer(std::vector<char>& vec);
};

#endif // __JJC__SERIALCONTROLLER__H__


