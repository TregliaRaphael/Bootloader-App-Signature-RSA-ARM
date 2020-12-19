#!/usr/bin/env python3.8

import signal
import sys
import struct
import os.path
from time import sleep

from serial import Serial, SerialException


flash_start = "#$FLASH_START"
flash_end = "#$FLASH_FINISH"
flash_erase = "#$ERASE_MEM"
flash_abort =  "#$FLASH_ABORT"



with open(sys.argv[1], 'rb') as binary:
    
    try:
        s = Serial(port='/dev/ttyACM1', baudrate=115200, bytesize=8, parity='N', stopbits=1, timeout=None, xonxoff=0, rtscts=0)
    except SerialException:
        print("Can't open serial")
        exit()


    def signal_handler(sig,frame):
        s.write(flash_abort.encode('ascii'))
        print(s.readline().decode('ascii'))
        sys.exit(0)
    signal.signal(signal.SIGINT, signal_handler)

    print("Enter 1 to flash")
    print("Enter 2 to erase flash memory")
    print("Or enter password")
    print("Enter q to exit")

    while True:
        val = sys.stdin.readline()
        if val == '1\n':
            s.write(flash_start.encode('ascii'))
            perm = s.readline()
            if perm == b'YE\n':
                print("Flash Started")
                while (byte := binary.read(4)):
                    s.write(byte)
                binary.seek(0)
                print("Flash Ended, lets jump to the App")
                s.write(flash_end.encode('ascii'))
                exit()
            else:
                print("Flash rejected, need to enter a (valid) password")
        elif val == '2\n':
            s.write(flash_erase.encode('ascii'))
            sleep(0.2)
        elif val == 'q\n':
            exit()
        else:
            pwd = val
            s.write(pwd.encode('ascii'))
            print(s.readline().decode('ascii'))
