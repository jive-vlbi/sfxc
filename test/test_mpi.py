#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id: test_mpi.py 251 2007-06-12 13:56:30Z kruithof $

import sys, os,time, filecmp;

# Load the ccf files for testing:
RC_FILE = os.path.join(os.environ.get('HOME'), ".sfxcrc")
if os.path.isfile(RC_FILE):
  execfile(RC_FILE)

status = os.system("make test_mpi")
if (status != 0): 
  sys.exit(1)

for control_file in controlfiles:
  status = os.system("mpirun -np 2 ./test_mpi " +
                     (" ".join(control_file))+" "+tmp_output_directory)
  if (status != 0):
    print "test_mpi: returned error."
    sys.exit(1);

sys.exit(0);
