#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id$

import sys, os,time;

infile = "data/input.txt"
infile = "/jop54_0/kruithof/data/JIVE-013/JIVE-13.del"
outfile = "output.txt"

status = os.system("compile test_Data_reader_TCP")
if (status != 0): sys.exit(1)

os.system("mpirun -np 2 ./test_Data_reader_TCP "+infile+" "+outfile);
if (status != 0): sys.exit(1)
