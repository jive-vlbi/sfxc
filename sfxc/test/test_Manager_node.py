#!/usr/bin/python

import sys, os,time, filecmp;

RC_FILE = os.path.join(os.environ.get('HOME'), ".sfxcrc")
if os.path.isfile(RC_FILE):
  execfile(RC_FILE)
  
sys.exit(0);
