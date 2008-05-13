#!/usr/bin/env python
import re
import numarray as na
from pylab import *

def do_picture(infile, outfile):
  figure(figsize=[10, 10])  
  infile = open(infile, "r")
  x0=0
  y0=0
  xs = []
  ys = []
  
  title("Real-time vs Computation time")
  xlabel("Computation time (in seconds)")
  ylabel("Real time (in seconds)")
   
   
  for line in infile:
    ret = re.match(".* PROGRESS t=([0-9]*): starting timeslice ([0-9]*)", line)     
    if ret:
      x = int(ret.groups()[0])/1000000
      y = int(ret.groups()[1])/1000.
    
      if x0 == 0:
        x0 = x
        y0 = y
      
      xs.append(x-x0),
      ys.append(y-y0)

  plot(xs,ys, color='b', label="sfxc runtime")
  plot(xs,xs, color='r', label="ideal real-time application")
  legend(loc=2) 
  savefig(outfile, dpi=50)


if len(sys.argv) < 3:
  print sys.argv[0],"<std_output.txt> <out.png>"
  sys.exit(-1)
    
do_picture(sys.argv[1], sys.argv[2])
