#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id$

import sys, os,time, filecmp;

# Load the ccf files for testing:
RC_FILE = os.path.join(os.environ.get('HOME'), ".sfxcrc")
if os.path.isfile(RC_FILE):
  execfile(RC_FILE)

# compile the executable
#status = os.system("cd ..; compile sfxc")
#if (status != 0): sys.exit(1)

status = os.system("make all")
if (status != 0): sys.exit(1)

# run the executable on all ccf files
for [ctrlfile,vexfile] in controlfiles:
  status = os.system("time mpirun -p4pg PI_with_input_readers manager_node "+
                     ctrlfile+" "+vexfile)
  if (status != 0): sys.exit(1)
  
sys.exit(0);