#!/usr/bin/python

# 2007, Developed by Nico Kruithof <Kruithof@jive.nl>

from vex_parser import Vex
import sys, pprint

verbose = False
vex_file_found = False
for arg in sys.argv[1:]:
  if arg == "--verbose":
    verbose = True
  else:
    vex_file_found = True

if not vex_file_found:
  print "usage: "+sys.argv[0]+" [--verbose] <vex-file>"
  sys.exit(1)


for vexfile in sys.argv[1:]:
  if not vexfile == "--verbose":
    vex_data = Vex(vexfile)
    if verbose:
      pprint.pprint(vex_data)
