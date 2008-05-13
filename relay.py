#! /usr/bin/env python 

###########################################
#
# send the data read from a file 
# to a distant site.
# 
##########################################
import socket, time, sys
import fileutils

filename = sys.stdin
#filename = "std_output.txt"
oldlocation = 0
HOST = '146.50.22.197'    # The remote host
PORT = 50007              # The same port as used by the server

if len(sys.argv) != 2:
  print "Usage: "+sys.argv[0]+" <host> <port> <filetorelay>"
  print "     : using default: "+HOST+" "+`PORT`+" stdin"
else:  
  HOST = sys.argv[1]
  #PORT = int(sys.argv[2])
  #filename = sys.argv[3]


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

#s.send(filename)
fp = fileutils.Incr_file(filename)

counter = 0
for line in fp.read():
  print line
  s.send(line)
  counter += 1 
  
  if counter%10 == 0:   
    time.sleep(0.2)


