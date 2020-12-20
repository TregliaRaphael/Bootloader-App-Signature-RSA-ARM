#!/usr/bin/env python3.8

import signal
import sys
import struct
import os.path
from time import sleep

from serial import Serial, SerialException


flash_start = "#$FLASH_START"
send_sha = "1"
init_pwd = "2"
flash_end = "#$FLASH_FINISH"
flash_erase = "#$ERASE_MEM"
flash_abort =  "#$FLASH_ABORT"



with open(sys.argv[1], 'rb') as sha:
    shaa = sha.read().decode('ascii')
    try:
        s = Serial(port='/dev/ttyUSB0', baudrate=115200, bytesize=8, parity='N', stopbits=1, timeout=None, xonxoff=0, rtscts=0)
    except SerialException:
        print("Can't open serial")
        exit()
    
    s.write(send_sha.encode('ascii'))
    s.readline() #consume OK from uC
    sleep(0.2)
    print("Send sha256 to uC")
    s.write(shaa[:-1].encode('ascii'))
    print(s.readline())
    
    sleep(0.5)

    s.write(init_pwd.encode('ascii'))
    s.readline() #consume OK from uC
    print("Password init (10 caractères): ")
    val = sys.stdin.readline()
    while len(val[:-1]) != 10:
        print("You miss, pwd need 10 caractères")
        val = sys.stdin.readline()
    s.write(val[:-1].encode('ascii'))
    print(s.readline())

    exit()
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
                while (byte := shaa.read(4)):
                    s.write(byte)
                shaa.seek(0)
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
