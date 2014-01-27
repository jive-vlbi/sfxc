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

  a = cfg['aips']
  AIPS.userno = a['user_nr']
  uvdata = AIPSUVData(a['name'].encode('ascii'), a['class'].encode('ascii'), a['disk'], a['seq'])
  if cfg['outname'].startswith('file://'):
    outname = cfg['outname'][7:]
  else:
    outname = cfg['outname']
  return outname, uvdata

def get_bp_old(uvdata,outname):
  bp = uvdata.table('BP', 0)
  nstation = bp.keywords['NO_ANT']
  nif = bp.keywords['NO_IF']
  npol = bp.keywords['NO_POL']
  nchan = bp.keywords['NO_CHAN']
  outfile = open(outname+'.bp', 'w')
  bp_table = [[[] for j in range(4+(npol-1)*2)] for i in range(nstation)] 
  #pdb.set_trace()
  date_obs = [int(t) for t in uvdata.header.date_obs.split('-')] # format : year-month-day
  mjd_obs = mjd(date_obs[2], date_obs[1], date_obs[0])
  # Read the bp table into memory. 
  for row in bp:
   ant_no = row.antenna-1
   bp_table[ant_no][0].append(row.time)
   bp_table[ant_no][1].append(row.interval)
   bp_table[ant_no][2].append(row.real_1)
   bp_table[ant_no][3].append(row.imag_1)
   if(npol == 2):
     bp_table[ant_no][4].append(row.real_2)
     bp_table[ant_no][5].append(row.imag_2)
  # Convert to numpy arrays
  for ant in bp_table:
    for i in range(4+(npol-1)*2):
      ant[i] = array(ant[i], dtype='float32')

  #A table to convert the AIPS station numbers to SFXC station number
  aips2sfxc = []
  sfxc_antennas = sorted(uvdata.antennas)
  for ant in uvdata.antennas:
    aips2sfxc.append(sfxc_antennas.index(ant))
  
  #write the BP to disk
  global_header = array([nstation, nif, npol, nchan, mjd_obs], dtype='int32')
  global_header.tofile(outfile)
  for ant_no, ant in enumerate(bp_table):
    header = array([aips2sfxc[ant_no],len(ant[0])], dtype='int32')
    header.tofile(outfile)
    for i in xrange(len(ant[0])):
      # The complex bandpass
      bp_1 = (ant[2][i] + 1j * ant[3][i])
      ant[0][i:i+1].tofile(outfile) # time
      ant[1][i:i+1].tofile(outfile) # interval
      bp_1.tofile(outfile)
      if npol == 2:
        bp_2 = (ant[4][i] + 1j * ant[5][i])
        bp_2.tofile(outfile)

def get_fg(uvdata,outname):
  fg = uvdata.table('FG', 0)
  nstation = bp.keywords['NO_ANT']
  nif = bp.keywords['NO_IF']
  npol = bp.keywords['NO_POL']
  nchan = bp.keywords['NO_CHAN']
  outfile = open(outname+'.bp', 'w')
  bp_table = zeros([nstation, npol, nchan*nif], dtype=complex64)
  weights = zeros([nstation, npol, nchan*nif], dtype=float32)
  #pdb.set_trace()
  # Read the bp table and do weighted sum of all bandpasses
  for row in bp:
    ant_no = row.antenna-1
    for i in range(nif):
      for c in range(nchan):
        j = i*nchan + c
        if row.weight_1[i] > 0.01:
          if row.real_1[j] < 20.:
            weights[ant_no][0][j] += row.weight_1[i]
            bp_table[ant_no][0][j] += (row.real_1[j] + 1j*row.imag_1[j])*row.weight_1[i]
        if (npol ==2) and (row.weight_2[i] > 0.01):
          if row.real_2[j] < 20.:
            weights[ant_no][1][j] += row.weight_2[i]
            bp_table[ant_no][1][j] += (row.real_2[j] + 1j*row.imag_2[j])*row.weight_2[i]
    if (ant_no == 3):
      #print row.time,' : ',weights[4][0][0:32]#,'\n', bp_table[4][0][0:32],'\n',row.weight_1
      print row.time,' : ', row.weight_1
      #pdb.set_trace()
  # normalize bandpass
  bp_table /= (weights + 1e-8)

  #A table to convert the AIPS station numbers to SFXC station number
  aips2sfxc = []
  sfxc_antennas = sorted(uvdata.antennas)
  for ant in uvdata.antennas:
    aips2sfxc.append(sfxc_antennas.index(ant))
    print 'ant = ', ant, ', idx = ', aips2sfxc[-1]
  
  #write the BP to disk
  global_header = array([nstation, nif, npol, nchan], dtype='int32')
  global_header.tofile(outfile)
  for ant_no, ant in enumerate(bp_table):
    header = array([aips2sfxc[ant_no]], dtype='int32')
    header.tofile(outfile)
    ant[0].tofile(outfile)
    if npol == 2:
      ant[1].tofile(outfile)

def get_bp(uvdata,outname):
  bp = uvdata.table('BP', 0)
  nstation = bp.keywords['NO_ANT']
  nif = bp.keywords['NO_IF']
  npol = bp.keywords['NO_POL']
  nchan = bp.keywords['NO_CHAN']
  outfile = open(outname+'.bp', 'w')
  bp_table = zeros([nstation, npol, nchan*nif], dtype=complex64)
  weights = zeros([nstation, npol, nchan*nif], dtype=float32)
  #pdb.set_trace()
  # Read the bp table and do weighted sum of all bandpasses
  for row in bp:
    ant_no = row.antenna-1
    for i in range(nif):
      for c in range(nchan):
        j = i*nchan + c
        if row.weight_1[i] > 0.01:
          if row.real_1[j] < 20.:
            weights[ant_no][0][j] += row.weight_1[i]
            bp_table[ant_no][0][j] += (row.real_1[j] + 1j*row.imag_1[j])*row.weight_1[i]
        if (npol ==2) and (row.weight_2[i] > 0.01):
          if row.real_2[j] < 20.:
            weights[ant_no][1][j] += row.weight_2[i]
            bp_table[ant_no][1][j] += (row.real_2[j] + 1j*row.imag_2[j])*row.weight_2[i]
    if (ant_no == 3):
      #print row.time,' : ',weights[4][0][0:32]#,'\n', bp_table[4][0][0:32],'\n',row.weight_1
      print row.time,' : ', row.weight_1
  #pdb.set_trace()
  # normalize bandpass
  bp_table /= (weights + 1e-8)

  #A table to convert the AIPS station numbers to SFXC station number
  aips2sfxc = []
  sfxc_antennas = sorted(uvdata.antennas)
  for ant in uvdata.antennas:
    aips2sfxc.append(sfxc_antennas.index(ant))
    print 'ant = ', ant, ', idx = ', aips2sfxc[-1]
  
  #write headers to disk
  global_header = array([nstation, nif, npol, nchan], dtype='int32')
  global_header.tofile(outfile)
  # Write the IF's to disk
  fq = uvdata.table('FQ', 0)
  f0 = uvdata.header.crval[2] # The reference frequency
  frequencies = around(array([f + f0 for f in fq[0].if_freq], dtype ='float64'))
  frequencies.tofile(outfile)
  # Store the bandwidths
  bandwidths = array(fq[0].total_bandwidth, dtype='float64')
  bandwidths.tofile(outfile)
  print frequencies
  print bandwidths
  #write the bandpasses to disk
  for ant_no, ant in enumerate(bp_table):
    #pdb.set_trace()
    header = array([aips2sfxc[ant_no]], dtype='int32')
    header.tofile(outfile)
    ant[0].tofile(outfile)
    if npol == 2:
      ant[1].tofile(outfile)

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

def mjd(day, month, year):
  a = (14-month)/12;
  y = year + 4800 - a;
  m = month + 12*a - 3;
  jdn = day + ((153*m+2)/5) + 365*y + (y/4) - (y/100) + (y/400) - 32045
  return int(jdn - 2400000.5)

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

outname, uvdata = parse_cli_args()
#get_cl(uvdata)
get_bp(uvdata, outname)
