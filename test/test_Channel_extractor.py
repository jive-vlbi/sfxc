#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id: test_Channel_extractor.py 278 2007-07-04 07:27:05Z kruithof $

import sys, os,time, filecmp;

# Load the ccf files for testing:
RC_FILE = os.path.join(os.environ.get('HOME'), ".sfxcrc")
if os.path.isfile(RC_FILE):
  execfile(RC_FILE)

if (os.environ.get("MAKE")):
  status = os.system('$MAKE test_Channel_extractor')
else:
  status = os.system("make test_Channel_extractor")
if (status != 0): 
  sys.exit(1)

for [ctrl_file,vex_file] in controlfiles:
  status = os.system("./test_Channel_extractor " + ctrl_file+" "+vex_file)
  if (status != 0):
    print "test_Channel_extractor: returned error."
    sys.exit(1);

sys.exit(0);
