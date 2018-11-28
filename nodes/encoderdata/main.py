import serial
from time import sleep
import struct
import numpy as np

# need to know where CPJL messages are
import sys
sys.path.append('../../../CPJL/lib/gen/messages/python')
sys.path.append('../../../CPJL/lib/python')

from CPJL import *
from Encoder import *

baudrate = 9600
ser = serial.Serial('/dev/arduino', baudrate, timeout = 1)
sleep(2)

distance1 = 0
distance2 = 0

encoder_obj = Encoder(CPJL("localhost", 14000), "encoder_data")

print('Looping...')
while True:
    ser.write('d'.encode('utf-8'))
    sleep(0.5)
    res = ser.read(11)
    #print("Response length: " + str(len(res)))
    st = struct.unpack("<cicic", res)

    d1 = st[1]
    d2 = st[3]

    # load data in the structure and send it on its way
    encoder_obj.right = d1
    encoder_obj.left = d2
    encoder_obj.putMessage()

    print("d1: " + str(encoder_obj.left) + ", d2: " + str(encoder_obj.right))

    sleep(0.5)
