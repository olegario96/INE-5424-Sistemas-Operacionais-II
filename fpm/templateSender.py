#!/usr/bin/env python3

import serial
import sys
import time
import os.path
import struct
import argparse
#import urllib.request as urllib2
import requests
import json

parser = argparse.ArgumentParser(description='EPOSMote III programmer')
parser.add_argument('-d','--dev', help='EPOSMote III device descriptor file', default='/dev/ttyACM0')
args = vars(parser.parse_args())

dev = args['dev']

def msg_checksum(msg):
    lrc = 0
    for i in msg:
        lrc += i
    return struct.pack('B', ((lrc ^ 0xff) + 1) & 0xff)

hex_offset = 0
sequence_number = 0
last_address = 0 # This script assumes increasing addresses. It will ignore addresses out of increasing order.

print("Waiting for", dev, "to appear")
while not os.path.exists(dev) or not os.access(dev, os.W_OK):
    pass

print(dev, "found, trying to open it")

mote = serial.Serial(baudrate = 115200, port=dev, timeout=3000000)
mote.close()
mote.open()

print(dev, "Opened.")

templateHigh = int(mote.read(1))
templateLow = int(mote.read(1))
print(templateHigh)
print(templateLow)
templateNum = (templateHigh << 8) | templateLow
print(templateNum)

fingers = [[0 for x in range(768)] for y in range(templateNum)]

for finger in xrange(0,templateNum):
    for byte in xrange(0,768):
    	fingers[finger][byte] = mote.read(1)

headers = {'Content-type': 'application/json'}
url = "http://hammerfall.g.olegario.vms.ufsc.br:5000"

for finger in xrange(0, templateNum): 
    response = requests.post(url,data=json.dumps(fingers[finger]), headers=headers)
    print(response.text)