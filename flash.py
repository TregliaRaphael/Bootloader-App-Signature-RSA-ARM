#!/usr/bin/env python3.8

import sys
import struct
import os.path
from time import sleep

from serial import Serial

#binName = sys.argv[1]

#with open(binName) as m:
 #   message = m.read()

s = Serial(port='/dev/ttyACM1', baudrate=115200, bytesize=8, parity='N', stopbits=1, timeout=None, xonxoff=0, rtscts=0)

flash_start = "#$FLASH_START"
flash_end = "#$FLASH_FINISH"
flash_erase = "#$ERASE_MEM"

while True:
    print("Enter 1 to flash")
    print("Enter 2 to erase flash memory")
    print("Enter q to exit")
    val = sys.stdin.readline()
    if val == '1\n':
        s.write(flash_start.encode('ascii'))
        sleep(0.2)
    elif val == '2\n':
        s.write(flash_erase.encode('ascii'))
        sleep(0.2)
    elif val == 'q\n':
        exit()

