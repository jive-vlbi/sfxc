#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id$

import sys, os,time, filecmp;

RC_FILE = os.path.join(os.environ.get('HOME'), ".sfxcrc")
if os.path.isfile(RC_FILE):
  execfile(RC_FILE)

status = os.system("compile test_Transfer_control_parameters")
if (status != 0): sys.exit(1)

for ctrlfile in controlfiles:
  print "Control file: "+ctrlfile
  status = os.system("mpirun -np 2 test_Transfer_control_parameters "+ctrlfile)
  if (status != 0): sys.exit(1)
  
sys.exit(0);
