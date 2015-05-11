#! /usr/local/bin/python3
import glob, os
import serial
import time
from collections import defaultdict
from queue import Queue
import json
from pprint import pprint
from xbee import XBee, ZigBee

# Constants
START_CH = '^'
END_CH = '\r'

# Globals
ser = None
xbee = None
rxQueue = defaultdict(Queue)

# run function, fn, on all items in iterable, it.
def each(fn, it):
    for i in it: fn(i)

# print and pass on, use for inline debugging
def inprint(x): print(x, end=''); return(x)

# rx data and queue into a chr buffer
def handle_data_rx(data):
    each(rxQueue[data['source_addr_long']].put, map(chr, data['rf_data']))
    print("    from: ", data['source_addr_long'], "raw", data['rf_data'])

def process_data():
    for addr in rxQueue:
        q = rxQueue[addr]

        # search for start char
        while not q.empty() and (q.queue[0] is not START_CH): q.get()

        # wait for full message (comes in blocks)
        if (not q.empty()) and (END_CH in q.queue):
            q.get() # pop start character

            # compile message
            ch = ""
            payload = ""
            while (ch is not END_CH) and not q.empty():
                ch = q.get()
                print(ch, end='')
                payload += ch # simple and not so inefficient after all
                # http://stackoverflow.com/questions/19926089/python-equivalent-of-java-stringbuffer

            print("rx:", payload)

def find_port():
    return "/dev/tty.usbserial-DA01I3FX"

def init():
    global xbee, ser
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
            time.sleep(0.1)
        except KeyboardInterrupt:
            print("quit")
            raise

def end():
    xbee.halt()
    ser.close()

if __name__ == "__main__":
    init()
    try:
        main()
    finally:
        end()
