#!/usr/bin/env python
import sys, datetime, pdb
import vex_parser, vex_time
from optparse import OptionParser

if sys.version_info > (2, 5):
  import json
else:
  import simplejson as json

def parse_clock_early(clock_early):
  line = clock_early.partition('=')[2].partition(':')
  valid = line[0].strip()
  line = line[2].partition(':')
  offset = float(line[0].partition('usec')[0])
  line = line[2].partition(':')
  epoch = line[0].strip()
  line = line[2].partition(';')
  rate = float(line[0])
  return valid, offset, epoch, rate

######################### Main #########################################
parser = OptionParser('%prog <input vexfile> <output vexfile> <clock offsets file>')
(options, args) = parser.parse_args()
if len(args) != 3:
  parser.error('invalid number of arguments')
vex_in_name = args[0]
vex_out_name = args[1]
ctrl_name = args[2]

try:
  ctrl = json.load(open(ctrl_name, "r"))
except StandardError, err:
  print "Error loading control file : " + str(err)
  sys.exit(1);

try:
  vex = vex_parser.Vex(vex_in_name)
  vex_in = open(vex_in_name, "r")
except StandardError, err:
  print "Error loading input vexfile : " + str(err)
  sys.exit(1);

try:
  vex_out = open(vex_out_name, "w")
except:
  print "Unable to open output vex file : " + str(err)
  sys.exit(1);

clock_keys = []
stations = ctrl['stations']
for s in stations :
  clock_keys.append(vex['STATION'][s]['CLOCK'])

offsets = []
rates = []
channels = ctrl['channels']
for s in stations:
  offset = 0
  rate = 0
  for c in channels:
    clock = ctrl['clocks'][s][c]
    # Weighted average of clock and rate
    offset += clock[0]*clock[3]
    rate += clock[1]*clock[3]
  offsets.append(offset)
  rates.append(rate)
# Copy contents of input vex file to output vex file up until the CLOCK section
line = vex_in.readline()
while (not line.strip().startswith('$CLOCK')) and (line != ''):
  vex_out.write(line)
  line = vex_in.readline()

vex_out.write(line)
done_parsing = False
while not done_parsing:
  #pdb.set_trace()
  line = vex_in.readline()
  if line == '' or line.strip().startswith('$'):
    done_parsing = True
    vex_out.write(line)
  elif line.strip().startswith('def '):
    s = line.partition('def ')[2].partition(';')[0]
    vex_out.write(line)
    try:
      idx = clock_keys.index(s)
      line = vex_in.readline()
      while line != '' and not line.strip().startswith('clock_early'):
        vex_out.write(line)
        line = vex_in.readline()
      valid, old_clock, old_epoch, old_rate = parse_clock_early(line)
      # Substracting to dates behaves a bit odd if the outcome would be a negative number
      if vex_time.get_time(old_epoch) >= vex_time.get_time(ctrl['epoch']):
        dt = (vex_time.get_time(old_epoch) - vex_time.get_time(ctrl['epoch'])).seconds
      else:
        dt = -(vex_time.get_time(ctrl['epoch']) - vex_time.get_time(old_epoch)).seconds
      new_clock = old_clock + offsets[idx] + rates[idx]*dt
      new_rate = old_rate + rates[idx]
      vex_out.write('    clock_early =  %s :  %.4f usec :  %s  :  %.3e; '%(valid, new_clock, old_epoch, new_rate))
      vex_out.write('* Modified on ' +  str(datetime.datetime.now()).partition('.')[0]+'\n')
      vex_out.write('* ' + line) # comment out old clock offsets
    except:
      pass
  else:
    vex_out.write(line)
# copy the rest of the input vexfile
lines = vex_in.readlines()
vex_out.writelines(lines)
