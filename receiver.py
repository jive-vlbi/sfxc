#! /usr/bin/env python 

# Echo server program
import socket



HOST = '146.50.22.197'    # Symbolic name meaning the local host
PORT = 50007              # Arbitrary non-privileged port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
conn, addr = s.accept()

print 'Connected by', addr

inited = False
filename = "cgi-bin/std_output_raw.txt"
fp = open(filename, "wt")

while True:
    data = conn.recv(1024)
    if data : 
      inited = True
      fp.write(data)     
      print "Writing to file: "+data
      fp.flush()      
      

conn.close()
