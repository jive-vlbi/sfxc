#!/usr/bin/python

import glob, os, sys

for fname in glob.glob("*.vex"):
  status = os.system("../src/parser "+fname)
  if (status != 0):
    print "parsing failed for: "+fname
    sys.exit(1);
  status = os.system("../src/parser.py "+fname)
  if (status != 0):
    print "parsing failed for: "+fname
    sys.exit(1);


sys.exit(0);
