#!/usr/bin/python

import sys, os,time, filecmp;

RC_FILE = os.path.join(os.environ.get('HOME'), ".sfxcrc")
if os.path.isfile(RC_FILE):
  execfile(RC_FILE)

status = os.system("compile test_Correlator_node")
if (status != 0): sys.exit(1)

for ctrlfile in controlfiles:
  print "Control file: "+ctrlfile
  status = os.system("mpirun -np 3 test_Correlator_node "+ctrlfile)
  if (status != 0): sys.exit(1)
  
sys.exit(0);
