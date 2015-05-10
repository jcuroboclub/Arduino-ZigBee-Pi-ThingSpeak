#! /usr/local/bin/python3
import glob, os
import serial
import time
from collections import defaultdict
from queue import Queue
from xbee import XBee, ZigBee

# Constants
START_CH = '!'

# Globals
ser = None
xbee = None
rxQueue = defaultdict(Queue)

# run function, fn, on all items in iterable, it.
def each(fn, it): [fn(i) for i in it]

# print and pass on, use for inline debugging
def inprint(x): print(x, end=''); return(x)

def handle_data_rx(data):
    each(rxQueue[data['source_addr_long']].put, map(chr, data['rf_data']))
    print(data['source_addr_long'], rxQueue[data['source_addr_long']].qsize())

def process_data():
    for addr in rxQueue:
        q = rxQueue[addr]

        # search for start char
        while (q.get() is not START_CH) and not q.empty(): pass

        # compile message
        ch = ""
        while (ch is not '\r') and not q.empty():
            ch = q.get()
            print(ch, end='')
        if not q.empty(): print()

def find_port():
    return "/dev/tty.usbserial-DA01I3FX"

def init():
    port = find_port()
    baud = 9600
    print("Listening on:", port, "at", baud, "baud")
    os.system("cat " + port) # This cleared the port for me, good luck charm

    ser = serial.Serial(port, 9600)
    xbee = ZigBee(ser, escaped=True, callback=handle_data_rx)

def main():
    while True:
        try:
            process_data()
            time.sleep(0.5)
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
