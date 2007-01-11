#!/usr/bin/python

import sys, os,time, filecmp;

inputfile = "data/input.txt"
outputfile = "output.txt"

result = os.system("mpirun -np 2 ./test_Data_node "+inputfile+" "+outputfile)
if result:
  print "test_Data_node: returned error."
  sys.exit(1);

result = filecmp.cmp(inputfile, outputfile)
if not result:
  print "Compare: files differ."
  sys.exit(1);

os.remove(outputfile)

sys.exit(0);
