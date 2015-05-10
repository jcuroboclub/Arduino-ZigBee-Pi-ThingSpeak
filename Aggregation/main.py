#! /usr/local/bin/python3
import glob, os
import serial
import time
from xbee import XBee, ZigBee

#avail_ports = glob.glob('/dev/tty.*')
#[print("[{0}] {1}".format(i, port)) for i, port in enumerate(avail_ports)]
port = "/dev/tty.usbserial-DA01I3FX" # port = avail_ports[int(input('select: '))]
print(port)
os.system("cat " + port) # This helped clear port for me
ser = serial.Serial(port, 9600)
xbee = ZigBee(ser, escaped=True, callback=print)

while True:
    try:
        time.sleep(0.001)
        #print(xbee.wait_read_frame())
        #print('.')
    except KeyboardInterrupt:
        break

xbee.halt()
serial_port.close()
