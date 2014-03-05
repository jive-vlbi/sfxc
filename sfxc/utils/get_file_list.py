#!/usr/bin/env python
# Produce a list sorted according to start time of all Mark5 / VDIF files matching a pattern
import sys, optparse, subprocess, datetime

def get_options():
  usage = "%prog [options] <filetype> <file1> <file2> ...\n" + \
          "with filetype=mark5a/mark5b/vlba/vdif"
  parser = optparse.OptionParser(usage = usage)
  parser.add_option("-y", "--year", dest="year", type="int", 
                    help="Year in which data was taken (to correct the dates\
                          from Mark5A / Mark5B / VLBA time stamps", metavar="YEAR")
  (options, args) = parser.parse_args()
  if len(args) < 2:
    parser.error("Invalid number of arguments")

  filetype = args[0].lower()
  filetypes = ['mark5a', 'mark5b', 'vlba', 'vdif']
  if filetype not in filetypes:
    parser.error("Invalid filetype : " + filetype)

  # Get list of files
  files = args[1:]
  if (filetype == 'mark5a') or (filetype == 'vlba'):
    for i, file in enumerate(files):
      files[i] = 'file://' + file
  return filetype, files, options.year

def parse_datastring(datestring):
  a = datestring.partition('y')
  year = int(a[0])
  a = a[2].partition('d')
  doy = int(a[0])
  a = a[2].partition('h')
  hour = int(a[0])
  a = a[2].partition('m')
  minute = int(a[0])
  a = a[2].partition('s')
  sec = int(a[0])
  time = datetime.datetime(year, 1, 1, hour, minute, sec)
  dt = datetime.timedelta(days=doy-1)
  return time + dt
#
######################
# Main Program
######################
#
filetype, files, year = get_options()

args = [filetype+'_print_headers', '-n', '1']
if (year != None) and (filetype != 'vdif'):
  args.append("-y")
  args.append(`year`)

parsed_files = []
for file in files:
  a = subprocess.Popen(args+[file], stdout=subprocess.PIPE)
  lines = a.stdout.readlines()
  a.wait()
  if a.returncode == 0:
    line = lines[-1].split()
    for item in line:
      try:
        time = parse_datastring(item)
        break # We are done searching
      except ValueError:
        pass
    parsed_files.append((file,time))

# Sort the files according to their start time
sorted_files = sorted(parsed_files, key=lambda f: f[1])
for t in sorted_files:
  if t[0][:7] == 'file://':
    print t[0], str(t[1])
  else:
    print 'file://'+t[0], str(t[1])
