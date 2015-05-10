#! /usr/local/bin/python3
import glob, os
import serial
import time
from xbee import XBee, ZigBee

ser = None
xbee = None

def find_port():
    return "/dev/tty.usbserial-DA01I3FX"

def init():
    port = find_port()
    baud = 9600
    print("Listening on:", port, "at", baud, "baud")
    os.system("cat " + port) # This cleared the port for me, good luck charm

    ser = serial.Serial(port, 9600)
    xbee = ZigBee(ser, escaped=True, callback=print)

def main():
    while True:
        try:
            time.sleep(0.001)
        except KeyboardInterrupt:
            break

def end():
    xbee.halt()
    serial_port.close()

if __name__ == "__main__":
    init()
    try:
        main()
    finally:
        end()
