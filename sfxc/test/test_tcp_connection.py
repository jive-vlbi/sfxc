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

status = os.system("make test_TCP_connection")
if (status != 0): sys.exit(1)

status = os.system("mpirun -np 2 test_TCP_connection ")
if (status != 0): sys.exit(1)
  
sys.exit(0);
