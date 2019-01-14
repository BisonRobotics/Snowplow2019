import serial
from time import sleep
import struct
import numpy as np
import matplotlib.pyplot as plt

plt.axis([0, 100, -10000, 10000])
ser = serial.Serial('COM3', 9600, timeout = 1)
sleep(2)

distance1 = 0
distance2 = 0

class ser:
    def __init__(self):
        ser = serial.Serial('COM3', 9600, timeout=1)


def update_distances():
    global distance1, distance2
    ser.write('d')
    response = ser.read(11)
    responseStruct = struct.unpack("<cicic", response)
    print(responseStruct)
    deltaDistance1 = responseStruct[1]
    deltaDistance2 = responseStruct[3]

    #check for errors
    if responseStruct[0] != 'd':
        print("Error in return char, expected: d Actual: " + str(responseStruct[0]))
    CRC1 = responseStruct[2]
    if CRC1 != responseStruct[2]:
        print("Error in CRC1, expectec: " + str(CRC1) + " Actaul: " + str(int(responseStruct[2])))

    CRC2 = responseStruct[4]
    if CRC2 != responseStruct[4]:
        print("Error in CRC2, expectec: " + str(CRC2) + " Actaul: " + str(int(responseStruct[4])))

    distance1 += deltaDistance1
    distance2 += deltaDistance2

for i in range(1000):
    update_distances()
    y = distance1
    plt.scatter(i, y)
    plt.pause(0.05)

plt.show()
ser.close()
