#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id$
# 

import sys, os,time, filecmp, getopt;

numProcesses = 15

def usage():
  print sys.argv[0]
  print "  -n   d"
  print "  --np d: number of processes"

try:
  opts, args = getopt.getopt(sys.argv[1:], "hn:", ["help", "np="])
except getopt.GetoptError:
  usage()
  sys.exit(2)

for opt, arg in opts:
  if opt in ("-h", "--help"):
    usage()
    sys.exit(2)
  if opt in ("-n", "--np"):
    numProcesses = arg;

# Load the ccf files for testing:
RC_FILE = os.path.join(os.environ.get('HOME'), ".sfxcrc")
if os.path.isfile(RC_FILE):
  execfile(RC_FILE)

# compile the executable
if (os.environ.get("MAKE")):
  status = os.system('$MAKE sfxc_with_input_nodes')
else:
  status = os.system("make sfxc_with_input_nodes")
if (status != 0): sys.exit(1)

# run the executable on all ccf files
for ctrlfile in controlfiles:
  print "Control file: "+ctrlfile
  status = os.system("time mpirun -np "+str(numProcesses)+" sfxc_with_input_nodes "+ctrlfile)
  if (status != 0): sys.exit(1)
  
sys.exit(0);
