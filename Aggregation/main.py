#! /usr/local/bin/python3
import os
import glob
import serial
import time
from collections import defaultdict
from queue import Queue
from copy import deepcopy
import re
import json
from pprint import pprint
import numpy as np
from xbee import XBee, ZigBee
import thingspeak as ts

# Parameters
api_keys = {
    37372: "BKSHA6VI5K13A6HL",
    38007: "8BXAPC55OULDOZ9O"
}

# Constants
START_CH = '^'
END_CH = '\r'

# Globals
ser = None
xbee = None
rxQueue = defaultdict(Queue)
channels = defaultdict(ts.channel)

# run function, fn, on all items in iterable, it.
def each(fn, it):
    for i in it: fn(i)

# print and pass on, use for inline debugging
def inprint(x): print(x, end=''); return(x)

# rx data and queue into a chr buffer
def handle_data_rx(data):
    each(rxQueue[data['source_addr_long']].put, map(chr, data['rf_data']))
    #print("    from: ", data['source_addr_long'], "raw", data['rf_data'])

def process_data():
    # rxQueue is subject to change, use list() in order to prevent
    # RuntimeError: dictionary changed size during iteration
    for addr in list(rxQueue):
        q = rxQueue[addr]

        # search for start char
        while not q.empty() and (q.queue[0] is not START_CH): q.get()

        # wait for full message (comes in blocks)
        if (not q.empty()) and (END_CH in q.queue):
            q.get() # pop start character

            # compile message
            ch = ""
            payload = ""
            while not q.empty() and (ch is not END_CH):
                ch = q.get()
                #print(ch, end='')
                payload += ch # simple and not so inefficient after all
                # http://stackoverflow.com/questions/19926089/python-equivalent-of-java-stringbuffer

            # add quotes to keys (proper JSON formatting)
            payload = re.sub('([{,])([^{:\s"]*):',
                             lambda m: '%s"%s":'%(m.group(1),m.group(2)),
                             payload)

            def group_in_channels(d):
                channels = np.unique(list(map(lambda x: x["channel"], d)))
                grouped = {}
                for c in channels:
                    fields = map(lambda x: x["field"],
                                 filter(lambda x: x["channel"] == c,
                                        d))
                    grouped[c] = [map(lambda x: x["data"],
                                     filter(lambda x: x["field"] == f,
                                            filter(lambda x: x["channel"] == c,
                                                   d))).__next__()
                                  if f in fields else None
                                  for f in range(1,9)]
                return grouped

            print(payload)
            try:
                payload = json.loads(payload)
            except ValueError as e:
                print("Corrupt Packet?")
                continue
            grouped = group_in_channels(payload)
            #print(grouped)
            for ch in grouped:
                if ch in channels:
                    channels[ch].update(grouped[ch])

def find_port():
    return "/dev/tty.usbserial-DA01I3FX"

def init():
    global xbee, ser, channels
    port = find_port()
    baud = 9600
    print("Listening on:", port, "at", baud, "baud")
    #os.system("cat " + port) # This cleared the port for me, good luck charm

    ser = serial.Serial(port, 9600)
    xbee = ZigBee(ser, escaped=True, callback=handle_data_rx)

    for ch in api_keys:
        channels[ch] = ts.channel(api_keys[ch], None)
    pprint(channels)

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
