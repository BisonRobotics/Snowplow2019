import serial
from time import sleep, time
import struct
import numpy as np

# need to know where CPJL messages are
import sys
sys.path.append('../../../CPJL/lib/gen/messages/python')
sys.path.append('../../../CPJL/lib/python')

from CPJL import *
from Encoder import *

baudrate = 115200
ser = serial.Serial('/dev/arduino', baudrate, timeout = 1)
sleep(2)

distance1 = 0
distance2 = 0

def crc8(buf, length):
    returnVal = 0
    for i in range(length):
        returnVal = ((buf[i] & 0xFF) ^ returnVal) & 0xFF
    return returnVal

def encoder_callback(obj):
    print("  RX: left: " + str(obj.left) + ", right: " + str(obj.right) + "\n")

encoder_receiver = Encoder(CPJL("localhost", 14000), "encoder_data", encoder_callback)
encoder_obj = Encoder(CPJL("localhost", 14000), "encoder_data")

loop = CPJL_Message.loop()
loop.start()

current_loop_iteration = int(0)

print('Looping...')
while True:
    # Request encoder data
    ser.write('d'.encode('utf-8'))

    # Read reply with data
    try:
        res = ser.read(11)
    except EnvironmentError:
        print("Error reading serial data!")
        continue

    # microsecond timestamp
    ts = time.time() * 1000000.0

    #unpack data
    #print("Response length: " + str(len(res)))
    st = struct.unpack("<cicic", res)

    # Check for CRC error
    error = False
    if st[0] != 'd':
        error = False
    if crc8(res, 5) != st[2]:
        error = False
    if crc8(res[6:10], 4) != st[4]:
        error = False

    while error:
        # Request data be re-sent
        print("CRC Error, requesting re-send")
        ser.write('r'.encode('utf-8'))
        try:
            res = ser.read(11)
        except EnvironmentError:
            print("Error reading serial data!")
            continue
        st = struct.unpack("<cicic", res)
        error = False
        if st[0] != 'r':
            error = True
        if crc8(bytearray(res), 5) != st[2]:
            error = True
        if crc8(bytearray(res[6:10]), 4) != st[4]:
            error = True

    d1 = st[1]
    d2 = st[3]

    # load data in the structure and send it on its way
    encoder_obj.left = int(-d1)
    encoder_obj.right = int(d2)
    encoder_obj.timestamp = int(ts)
    encoder_obj.putMessage()

    print("  TX: left: " + str(encoder_obj.left) + ", right: " + str(encoder_obj.right))
    current_loop_iteration = current_loop_iteration + 1
    sleep(0.1)
