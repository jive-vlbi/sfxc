#!/usr/bin/python

# Copyright (c) 2009 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Aard Keimpema <keimpema@jive.nl>
# 

import sys, os
import subprocess
from optparse import OptionParser

# The json module is new in Python 2.6; fall back on simplejson if it
# isn't available.
try:
    import json
except:
    import simplejson as json
    pass

def gen_delay_tables(vex_file, ctrl_file, nprocs):
  if not os.path.exists(vex_file):
    print "Vex file does not exist : " + vex_file
    sys.exit(1)

  if not os.path.exists(ctrl_file):
    print "Control file does not exist : " + ctrl_file
    sys.exit(1)

  # compute the number of processes needed from the control file
  try:
    ctrl = json.load(open(ctrl_file, "r"))
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
  nstations = len(ctrl["stations"])
  if nprocs == None:
    nprocs = nstations
  else:
    nprocs = min(nstations, nprocs)
  procs = [(None, None) for i in range(nprocs)]
  for i in range(nprocs):
    station = ctrl["stations"][i]
    procs[i] = (station, gen_delay(delay_directory, experiment_name, vex_file, station))
  for i in range(nprocs, nstations):
    station = ctrl["stations"][i]
    done = False
    while not done:
      for j in range(nprocs):
        status = procs[j][1].poll()
        if status != None:
          if status < 0:
            print "Error failed generating delay file for station", procs[j][0]
            sys.exit(1)
          else:
            done = True
            procs[j] = (station, gen_delay(delay_directory, experiment_name, vex_file, station))
            break
  # Wait for processes to finish
  for j in range(nprocs):
    status = procs[j][1].wait()
    if status < 0:
      print "Error failed generating delay file for station", procs[j][0]

def gen_delay(delay_directory, experiment_name, vex_file, station):
  delay_file = delay_directory + "/" + experiment_name + "_" + station + ".del"
  cmd = ["generate_delay_model", vex_file, station , delay_file]
  print cmd
  proc = subprocess.Popen(cmd)
  print "Generating delay model for station " + station
  return proc


######### MAIN CODE
if __name__ == "__main__":
  usage = "Usage: %prog [options] <vex file> <control file>"
  parser = OptionParser(usage=usage)
  parser.add_option("-p", "--number-process", dest="nprocs", 
                    help="Specify the number of generate jobs to be run simultaneously",
                    action="store", type="int")
  opts, args = parser.parse_args()
  if len(args) == 2:
    gen_delay_tables(args[0], args[1], opts.nprocs)
  else:
    parser.error("Invalid number of arguments")
