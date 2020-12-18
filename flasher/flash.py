#!/usr/bin/env python3.8

import sys
import struct
import os.path
from time import sleep

from serial import Serial

flash_start = "#$FLASH_START"
flash_end = "#$FLASH_FINISH"
flash_erase = "#$ERASE_MEM"


with open(sys.argv[1], 'rb') as binary:
    

    s = Serial(port='/dev/ttyACM1', baudrate=115200, bytesize=8, parity='N', stopbits=1, timeout=None, xonxoff=0, rtscts=0)

    while True:
        print("Enter 1 to flash")
        print("Enter 2 to erase flash memory")
        print("Or enter password")
        print("Enter q to exit")
        val = sys.stdin.readline()
        if val == '1\n':
            s.write(flash_start.encode('ascii'))
            perm = s.readline()
            if perm == b'YE\n':
                while (byte := binary.read(4)):
                    s.write(byte)
                binary.seek(0)
                s.write(flash_end.encode('ascii'))
                sleep(0.2)
        elif val == '2\n':
            s.write(flash_erase.encode('ascii'))
            sleep(0.2)
        elif val == 'q\n':
            exit()
        else:
            pwd = val
            s.write(pwd.encode('ascii'))
            print(s.readline().decode('ascii'))
