#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id: test_Channel_extractor.py 278 2007-07-04 07:27:05Z kruithof $


import sys, os, time, filecmp;

def which (filename):
  if not os.environ.has_key('PATH') or os.environ['PATH'] == '':
    p = os.defpath
  else:
    p = os.environ['PATH']

  pathlist = p.split (os.pathsep)

  for path in pathlist:
    f = os.path.join(path, filename)
    if os.access(f, os.X_OK):
      return f
  return None


######### MAIN CODE

if len(sys.argv) == 1:
  print "Usage", sys.argv[0], " <vex-file>"
  sys.exit(1)

vex_file = sys.argv[1]
(base,ext) = os.path.splitext(os.path.basename(vex_file))
ctrl_file = base+".ctrl"

# check if ctrl file exists, and create it if not
if not os.path.exists(ctrl_file):
  status = os.system("vex2ccf "+vex_file+" "+ctrl_file)
  if (status != 0):
    print "vex2ccf: returned error."
    sys.exit(1);

  # allow the user to edit the ctrl file
  status = os.system("emacs "+vex_file+" "+ctrl_file)
  if (status != 0):
    print "emacs: returned error."
    sys.exit(1);

# allow the user to set the number of nodes
print "enter the number of nodes, 10 by default"
try:
  number_of_processes = int(sys.stdin.readline())
except ValueError:
  number_of_processes = 10

# run the software correlator
cmd="mpirun -np "+str(number_of_processes)+" "+which("sfxc")+\
     " "+ctrl_file+" "+vex_file
print cmd
status = os.system(cmd)
if (status != 0):
  print "sfxc: returned error."
  sys.exit(1);

# run the html generator
cmd = "produce_html_plotpage "+ctrl_file+" "+vex_file
print cmd
status = os.system(cmd)
if (status != 0):
  print "produce_html_plotpage: returned error."
  sys.exit(1);

sys.exit(0)
