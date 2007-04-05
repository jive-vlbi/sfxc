#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id$

import sys, os,time;

infile = "/jop51_0/kruithof/data/JIVE-13.del"
infile = "data/input.txt"
outfile = "output.txt"

status = os.system("make test_Data_reader_mark5")
if (status != 0): sys.exit(1)

os.system("./test_Data_reader_mark5 "+outfile+"&");
#os.system("Net2file -f "+outfile+"&")

time.sleep(.1)
os.system("File2net -f"+infile)

#print " - Performing diff between files"
time.sleep(.1)
os.system("sync; diff "+infile+" "+outfile)

#print " - Removing output file"
os.remove(outfile)
