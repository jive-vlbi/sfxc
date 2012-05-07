#!/usr/bin/env ParselTongue
import AIPS, sys, datetime, struct, pdb
from Wizardry.AIPSData import AIPSUVData
from Wizardry.AIPSData import AIPSTableRow
from numpy import *
from scipy import interpolate
from optparse import OptionParser

try:
  import json
except:
  import simplejson as json

def parse_cli_args():
  parser = OptionParser('%prog <config_file>')
  (options, args) = parser.parse_args()
  if len(args) != 1:
    parser.error('invalid number of arguments')

  try:
    cfg = json.load(open(args[0], 'r'))
  except StandardError, err:
    print "Error loading config file : " + str(err)
    sys.exit(1);

  try:
    delay_directory = cfg['delay_directory']
    if delay_directory.lower().startswith('file://'):
      delay_directory = delay_directory[7:]
      
    if delay_directory == "":
      delay_directory = "./"
  except KeyError:
    delay_directory = "./"

  delays = {}
  for s in cfg['stations']:
    filename = delay_directory + '/' + cfg['exper_name'] + '_' + s + '.del'
    print 'open delay for station ', s, ' : file = ', filename
    file = open(filename, 'r+')
    hsize = struct.unpack('i', file.read(4))[0]
    header = file.read(hsize)
    delays[s.upper()] = file

  a = cfg['aips']
  AIPS.userno = a['user_nr']
  uvdata = AIPSUVData(a['name'].encode('ascii'), a['class'].encode('ascii'), a['disk'], a['seq'])
  return delays, uvdata

def get_cl(uvdata):
  cl = uvdata.table('CL', 0)
  nstation = len(uvdata.antennas)
  #scan = [[[]]*2]*nstation
  scan = [[[] for j in range(2)] for i in range(nstation)] 
  #pdb.set_trace()
  cal = []
  start_obs = [int(t) for t in uvdata.header.date_obs.split('-')]
  t0 = datetime.datetime(start_obs[0], start_obs[1], start_obs[2])
  time = [cl[0].time]
  source_old = cl[0].source_id 
  for row in cl:
    # for now we do only a single channel
    if row.freq_id == 1:
      #pdb.set_trace()
      dt = row.time-time[-1]
      if (dt > 2*row.time_interval) or (row.source_id != source_old):
        cal.append({'time':time, 'cal_table':scan})
        source_old = row.source_id
        scan = [[[] for j in range(2)] for i in range(nstation)] 
        time = []
      if dt > 0:
        time.append(row.time)
      amplitude = sqrt(row.real1[0]**2+row.imag1[0]**2)
      phase = arctan2(row.imag1[0], row.real1[0])
      scan[row.antenna_no-1][0].append(amplitude)
      scan[row.antenna_no-1][1].append(phase)
  cal.append({'time':time, 'cal_table':scan})
  return cal

def mjd2date(mjd):
  # algorithm taken from wikipedia
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
  return Y,M,D

def get_cal(scan, station):
  t = scan['time']
  ampl = scan['cal_table'][station][0]
  phase = scan['cal_table'][station][1]
  ampl_spline = interpolate.splrep(t, ampl,s=0)
  phase_spline = interpolate.splrep(t, phase,s=0)
  return t, ampl_spline, phase_spline

def apply_cal(delays, uvdata, cal):
  if len(cal) == 0:
    return
  start_obs = [int(t) for t in uvdata.header.date_obs.split('-')]
  t0 = datetime.datetime(start_obs[0], start_obs[1], start_obs[2])
  for station, delay in delays.iteritems():
    scan = 0
    s = uvdata.antennas.index(station.upper())
    t, ampl, phase  = get_cal(cal[scan], s)
    print 'station = ', station, ' ; nr =', s, ', cal_t[0] = ', t[0]
    source = delay.read(81)
    continue_search = True
    while (source != "") and continue_search:
      mjd = struct.unpack('i', delay.read(4))[0]
      Y,M,D = mjd2date(mjd)
      #pdb.set_trace()
      scan_day = datetime.datetime(Y,M,D)
      # format : time u v w delay phase ampl
      line = struct.unpack('7d', delay.read(56) )
      ts = scan_day + datetime.timedelta(seconds=line[0]) - t0
      taips = ts.days + ts.seconds / (24*60*60.)
      # The last point of each scan can be the first point of the next scan
      # Test if we need to go to the next scan
      while t[-1] <= taips + 1 / (24*60*60.):
        scan += 1
        if scan >= len(cal):
          continue_search = False
          break
        t, ampl, phase  = get_cal(cal[scan])
      #print 'line[0]', line[0]
      while (line[4] < 0) and continue_search:
        ts = scan_day + datetime.timedelta(seconds=line[0]) - t0
        taips = ts.days + ts.seconds / (24*60*60.)
        if t[-1] < taips - 0.5 / (24*60*60.):
          scan += 1
          if scan >= len(cal):
            continue_search = False
            break
          t, ampl, phase  = get_cal(cal[scan], s)
        if taips - 1 / (24*60*60.) >= t[0]:
          a = interpolate.splev(taips, ampl)
          p = interpolate.splev(taips, phase)
          print line[0],', a= ', a, ', p = ', p
          buf = struct.pack('2d',p,a)
          delay.seek(-16,1)
          delay.write(buf)
        line = struct.unpack('7d', delay.read(56))
        #print 'line[0]', line[0]
      source = delay.read(81)

###############################################3
######
###### 

delays, uvdata = parse_cli_args()
cal = get_cl(uvdata)
apply_cal(delays, uvdata, cal)
