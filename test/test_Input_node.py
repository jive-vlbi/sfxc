#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id$

import sys, os,time, filecmp;

inputfile = "/jop54_0/kruithof/data/n05c2/mark5/n06c2_da193_no0005.Mc"
inputfile = "data/input.txt"
outputfile = "output.txt"

status = os.system("make test_Input_node")
if (status != 0): 
  sys.exit(1)

status = os.system("mpirun -np 4 ./test_Input_node "+inputfile+" "+outputfile)
if (status != 0):
  print "test_Input_node: returned error."
  sys.exit(1);

os.system("sync")
status = filecmp.cmp(inputfile, outputfile)
if (status == 0):
  print "Compare: files differ."
  sys.exit(1);

os.remove(outputfile)

sys.exit(0);
