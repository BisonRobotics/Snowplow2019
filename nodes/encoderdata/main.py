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

def encoder_callback(obj):
    print("  RX: left: " + str(obj.left) + ", right: " + str(obj.right) + "\n")

encoder_receiver = Encoder(CPJL("localhost", 14000), "encoder_data", encoder_callback)
encoder_obj = Encoder(CPJL("localhost", 14000), "encoder_data")

loop = CPJL_Message.loop()
loop.start()

current_loop_iteration = int(0)

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
    encoder_obj.left = d1
    encoder_obj.right = d2
    encoder_obj.putMessage()

    print("  TX: left: " + str(encoder_obj.left) + ", right: " + str(encoder_obj.right))
    current_loop_iteration = current_loop_iteration + 1

    #sleep(0.05)
