#!/usr/bin/env python
import optparse, datetime
from struct import *
from math import pi, floor

def get_datetime(mjd, seconds):
  J = int(mjd + 2400001)
  j = J + 32044
  g = j / 146097; dg = j % 146097
  c = (dg / 36524 + 1) * 3 / 4; dc = dg - c*36524
  b = dc/1461 ;  db = dc % 1461;
  a = (db / 365 + 1) * 3 / 4 ; da = db-a*365
  y = g*400 + c*100 + b*4 + a
  m = (da*5 + 308)/153 - 2
  d = da-(m + 4)*153/5 + 122
  Y = y-4800 + (m + 2) / 12
  M = (m + 2) % 12 + 1
  D = d + 1
  date = datetime.datetime(Y,M,D)
  delta = datetime.timedelta(seconds=seconds)
  return date + delta

usage = "usage: %prog <delay file> <LO offset [Hz]>"
parser = optparse.OptionParser(usage=usage)
(options, args) = parser.parse_args()
if len(args) != 2:
  parser.error('Incorrect number of arguments')

delayfile = open(args[0], 'r+b')
offset = float(args[1])
# Read header
header_size = unpack('i', delayfile.read(4))[0]
header = delayfile.read(header_size)

new_scan = True
scan_nr = 0
while True:
  if new_scan:
    source = delayfile.read(81)
    if len(source) != 81:
      # done reading
      break 
    mjd = unpack('i', delayfile.read(4))[0]
    line = unpack('7d', delayfile.read(56))
    t = get_datetime(mjd, line[0])
    new_scan = False
    scan_nr += 1
    if scan_nr == 1:
      # Save the start time of the first scan
      tstart = t
    t0 = t
    # Conmpute the phase at the start of the scan modulo 2*pi
    delta = (t-tstart)
    seconds = delta.seconds + 86400*delta.days
    phase0 = seconds*(offset-floor(offset))
    phase0 = -2*pi*(phase0-floor(phase0))
  else:
    line = unpack('7d', delayfile.read(56))
    t = get_datetime(mjd, line[0])
  if line[0]==line[1]==line[2]==line[3]==line[4] == 0:
    new_scan = True
  else:
    # We compute the phase relative to the start of the experiment but 
    # at the start of each scan the phase is taken module 2*pi and then 
    # increases linearly within the scan. This is done to prevent 
    # issues with possible loss of floating point precision.
    delta = (t-t0)
    seconds = delta.seconds + 86400*delta.days
    phase = -2*pi*seconds*offset + phase0
    delayfile.seek(-16,1)
    delayfile.write(pack('2d',phase,line[6]))
