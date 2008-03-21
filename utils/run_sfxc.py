#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id: test_Channel_extractor.py 278 2007-07-04 07:27:05Z kruithof $


import sys, os, time, filecmp
import simplejson

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

def run_sfxc(vex_file, ctrl_file):
  if not os.path.exists(vex_file):
    print "Vex file does not exist"
    sys.exit(1)

  #construct the ctrl-filename from the vex file-name
  if ctrl_file == "":
    ctrl_file = os.path.splitext(os.path.basename(vex_file))[0]+".ctrl"

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

  # compute the number of processes needed from the control file
  ctrl = simplejson.load(open(ctrl_file, "r"))
  number_of_processes = 3 + len(ctrl["channels"]) + len(ctrl["stations"])

  # run the software correlator
  cmd="time mpirun -np "+str(number_of_processes)+" "+which("sfxc")+ \
      " "+ctrl_file+" "+vex_file+" | tee std_output.txt | grep -v PROGRESS"
  print cmd
  status = os.system(cmd)
  if (status != 0):
    print "sfxc: returned error."
    sys.exit(1);

    # check if the output file is empty
    output_file = ctrl["output_file"][7:]
    if not os.path.exists(output_file):
      sys.exit(1)
    if os.stat(output_file)[6] == 0:
      sys.exit(1)

  html_directory = "."
  if ctrl.has_key("html_output"):
    html_directory = ctrl["html_output"]
    if html_directory[:7] != "file://":
      print "html_output should start with file://"
    else:
      html_directory = html_directory[7:]
  elif os.path.exists("../html"):
    html_directory = "../html";

  output_file = ctrl["output_file"]
  if output_file[:7] != "file://":
    print "output_file should start with file://"
    sys.exit(1)
  else:
    output_file = output_file[7:]
    
  # run the html generator
  cmd = "produce_html_plotpage "+vex_file+" "+output_file+" "+html_directory
  print cmd
  status = os.system(cmd)

  if (status != 0):
    print "produce_html_plotpage: returned error."
    sys.exit(1)


######### MAIN CODE
if __name__ == "__main__":
  if len(sys.argv) == 2:
    run_sfxc(sys.argv[1], "")
  elif len(sys.argv) == 3:
    run_sfxc(sys.argv[1], sys.argv[2])
  else:
    print "Usage", sys.argv[0], " <vex-file> [<ctrl-file>]"
    sys.exit(1)

  sys.exit(0)
