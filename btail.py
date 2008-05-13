#! /usr/bin/env python 

###########################################
#
# Better tail
# 
##########################################
import socket, time, sys
import fileutils

filename = sys.stdin

fp = sys.stdin

counter = 0
line = fp.read(100000)
print line,
while True:
  line = fp.readline()
  if not line == None :
    print line,  
  time.sleep(0.01)


