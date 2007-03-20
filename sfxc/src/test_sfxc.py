#!/usr/bin/python

import sys, os,time, filecmp;

# Load the ccf files for testing:
RC_FILE = os.path.join(os.environ.get('HOME'), ".sfxcrc")
if os.path.isfile(RC_FILE):
  execfile(RC_FILE)

# compile the executable
status = os.system("compile sfxc")
if (status != 0): sys.exit(1)

# run the executable on all ccf files
for ctrlfile in controlfiles:
  print "Control file: "+ctrlfile
  status = os.system("sfxc "+ctrlfile)
  if (status != 0): sys.exit(1)
  
sys.exit(0);
