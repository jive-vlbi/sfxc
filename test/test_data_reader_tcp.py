#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id$

import sys, os,time;

infile = "/jop54_0/kruithof/data/JIVE-013/JIVE-13.del"
infile = "data/input.txt"
outfile = "output.txt"

status = os.system("make test_data_reader_tcp")
if (status != 0): sys.exit(status)

status = os.system("mpirun -np 2 ./test_data_reader_tcp "+infile+" "+outfile);
print status;
if (status != 0): sys.exit(status)
