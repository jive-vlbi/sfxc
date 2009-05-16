#!/usr/bin/python

# Copyright (c) 2009 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Aard Keimpema <keimpema@jive.nl>
# 

import sys, os, time, filecmp
import simplejson

def gen_table(vex_file, ctrl_file):
  if not os.path.exists(vex_file):
    print "Vex file does not exist : " + vex_file
    sys.exit(1)

  if not os.path.exists(ctrl_file):
    print "Control file does not exist : " + ctrl_file
    sys.exit(1)

  # compute the number of processes needed from the control file
  try:
    ctrl = simplejson.load(open(ctrl_file, "r"))
  except StandardError, err:
    print "Error loading control file : " + str(err)
    sys.exit(1);

  if ctrl.has_key("delay_directory"):
    delay_directory = ctrl["delay_directory"]
    if delay_directory[:7] != "file://":
      print "WARNING : delay_directory should start with file://"
    elif delay_directory == "file://":
      delay_directory = "."
    else:
      delay_directory = delay_directory[7:]
  else:
    delay_directory = "/tmp"
  
  if not ctrl.has_key("stations"):
    print "Control file does not have a stations field"
    exit(0)
  experiment_name = ctrl["exper_name"]
  
  for s in ctrl["stations"]:
    delay_file = delay_directory + "/" + experiment_name.upper() + "_" + s + ".del"
    cmd = "generate_delay_model " + vex_file + " " + s + " " + delay_file
    print "Generating delay model for station " + s
    if(os.system(cmd)!=0):
      print "Error executing command : " + cmd
      sys.exit(1)


######### MAIN CODE
if __name__ == "__main__":
  if len(sys.argv) == 3:
    gen_table(sys.argv[1], sys.argv[2])
  else:
    print "Usage", sys.argv[0], " <vex-file> <ctrl-file>"
    sys.exit(1)

  sys.exit(0)
