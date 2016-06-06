#!/usr/bin/env python
from vex import Vex
from optparse import OptionParser
from numpy import *
from scipy.interpolate import UnivariateSpline
import struct, time ,os
import pdb

# Size of headers in correlation file
size_timeslice = 16 # 4 integers
size_uvw = 32 # 2 integers and 3 doubles
size_baseline = 8 # 1 integer and 4 char (one char padding)
size_statistics = 24 # 4 chars, 5 integers

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

def mjd2date(mjd, sec):
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
  tupletime = time.strptime("%d %d %d"%(Y, M, D), "%Y %m %d")  
  return time.mktime(tupletime) + sec

def load_delay_table(filename):
  f = open(filename, 'r')
  hsize = struct.unpack('i', f.read(4))[0]
  header = f.read(hsize)
  source = f.read(81).rstrip('\0 ')
  delay_table = {}
  while source != "":
    mjd = struct.unpack('i', f.read(4))[0]
    line = struct.unpack('7d', f.read(56))
    start_scan = mjd2date(mjd, line[0])
    try:
      scan = delay_table[start_scan]
    except KeyError:
      scan = {"pointing_centre": source, "table": {}}
      delay_table[start_scan] = scan 
    t = []
    u = []
    v = []
    w = []
    delay = []
    t0 = mjd2date(mjd, 0) # midnight reference day
    while (line[0] != 0) and (line[4] != 0):
      t.append(t0 + line[0])
      u.append(line[1])
      v.append(line[2])
      w.append(line[3])
      delay.append(line[4])
      line = struct.unpack('7d', f.read(56))
    if source == scan["pointing_centre"]:
      scan["times"] =  array(t)
      scan["tstart"] = start_scan
      scan["tend"] = start_scan + 1. + (t[-1] - t[0])
    elif (scan["times"] != t).any():
      print 'Error: times for source', source, 'in the scan starting at t = ',start_scan,\
            "don't match that of the pointing centre"
      sys.exit(1)
    scan["table"][source] = {}
    scan["table"][source]["u"] = array(u)
    scan["table"][source]["v"] = array(v)
    scan["table"][source]["w"] = array(w)
    scan["table"][source]["delay"] = array(delay)
    source = f.read(81).rstrip('\0 ')
  return delay_table

def get_splines(delays, stations, t):
  splines = {}
  for station in delays:
    nr = stations.index(station)
    # Check if station is present in scan
    table = None
    print station
    for scankey in delays[station]:
      scan = delays[station][scankey]
      print 't=',t,', start=', scan["tstart"], ', end=', scan["tend"]
      if (scan["tstart"] <= t) and (scan["tend"] > t):
        pointing_centre = scan["pointing_centre"]
        table = scan["table"]
        break
    if table != None:
      for source in table:
        model = table[source]
        spline = {}
        spline["u"] = UnivariateSpline(scan["times"], model["u"], s=0, k=3)
        spline["v"] = UnivariateSpline(scan["times"], model["v"], s=0, k=3)
        spline["w"] = UnivariateSpline(scan["times"], model["w"], s=0, k=3)
        spline["delay"] = UnivariateSpline(scan["times"], model["delay"], s=0, k=3)
        spline["rate"] = spline["delay"].derivative()
        #pdb.set_trace()
        print "Add spline for source", source ,", station =", station, ", nr =", nr
        try:
          splines[source][nr] = spline
        except KeyError:
          splines[source] = {nr: spline}
  return splines, pointing_centre

def get_frequencies(vex, setup_station, t):
  # find mode for current scan by creating a sorted list of scans, this would
  # be simpler if we assumed the scans to already be sorted in time
  scans = []
  for scan in vex['SCHED']:
    tstart = vex2time(vex['SCHED'][scan]['start'])
    mode = vex['SCHED'][scan]['mode']
    scans.append((tstart,mode))
  scans.sort()
  for val in scans:
    if (t >= val[0]):
      mode = val[1]
      break
  # Get frequency section
  for f in vex['MODE'][mode].getall('FREQ'):
    if setup_station in f[1:]:
      freq = f[0]
      break
  # Get the frequencies themselves
  channels = set()
  for chan in vex['FREQ'][freq].getall("chan_def"):
    f = float(chan[1].split()[0])*1e6 # Sky frequency in Hz
    sb = 0 if (chan[2].upper() == 'L') else 1
    bw = float(chan[3].split()[0])*1e6 # Bandwidth in Hz
    channels.add((f,sb,bw))
  channels = sorted([f for f in channels])
  frequencies = []
  prev = -1
  for (f, sb, bw) in channels:
    if (f != prev):
      frequencies.append([(),()])
      prev = f
    frequencies[-1][sb] = (f, bw)
    print 'freq_nr=',len(frequencies)-1,', sb=', sb, ', f=', f,', bw =', bw
  return frequencies

def read_global_header(corfile):
  corfile.seek(0)
  global_header_size = struct.unpack('i', corfile.read(4))[0]
  corfile.seek(0)
  global_header_bin = corfile.read(global_header_size)
  if global_header_size == 64:
    gheader = struct.unpack('i32s2h5ib3c',global_header_bin[:64])
  else:
    gheader = struct.unpack('i32s2h5ib15s',global_header_bin[:76])
  global_header = {'start_year': gheader[2], 'start_day': gheader[3], 'start_time': gheader[4]}
  n = gheader[1].index('\0')
  global_header['experiment'] = gheader[1][:n]
  global_header['number_channels'] = gheader[5]
  global_header['integration_time'] = gheader[6]
  global_header['polarization'] = gheader[9]
  return global_header, global_header_bin

def scan2timeslice(vex, global_header):
  year = global_header['start_year']
  day = global_header['start_day']
  seconds = global_header['start_time']
  start_time_tuple = time.strptime("%d %d"%(year, day), "%Y %j")
  start_time = time.mktime(start_time_tuple) + seconds
  # Make sorted list of start times of all scans
  scans = []
  for scan_name in vex['SCHED']:
    scan = vex['SCHED'][scan_name]
    scan_start = vex2time(scan['start'])
    scan_length = 0
    for station in scan.getall('station'):
      length = int(station[2].split()[0])
      if (length > scan_length):
        scan_length = length
    scans.append((scan_start, scan_length))
  scans.sort()
  nscans = len(scans)

  # Find scan belonging to the first timeslice
  i=0
  while (i < nscans-1) and (scans[i+1][0] <= start_time):
    i += 1
  #pdb.set_trace() 
  inttime = global_header['integration_time']
  dt = int(scans[i][1] - round(start_time-scans[i][0])) * 1000000.
  n = int(ceil(dt / inttime))
  timeslices = [(0, start_time)]
  while (i < nscans-1):
    timeslices.append((n, scans[i+1][0]))
    i += 1
    n += int(ceil(scans[i][1] / inttime))
  return timeslices
    
def get_args():
  usage = '%prog <vex file> <correlation file> <delay_directory> <setup station>'
  parser = OptionParser(usage=usage)
  opts, args = parser.parse_args()
  if len(args) != 4:
    parser.error('Incorrect number of arguments')  
  vex = Vex(args[0])
  corfile = open(args[1], 'r')
  delay_dir = args[2]
  setup_station = args[3]
  stations =sorted([x for x in vex['STATION']])
  exper = vex['EXPER'].items()[0][1]['exper_name']
  # Load all available delay tables
  delays = {}
  for station in stations:
    filename = delay_dir+'/'+exper+'_'+station+'.del'
    try:
      delays[station] = load_delay_table(filename)
    except IOError:
      # There is no delay table for this station
      pass

  return vex, corfile, delays, setup_station, stations


#################################
#### MAIN
#################################

# Proper time.
os.environ['TZ'] = "UTC"

vex, corfile, delays, setup_station, stations = get_args()

global_header, global_header_bin = read_global_header(corfile)
timeslices = scan2timeslice(vex, global_header)

outfiles = {}
current_scan = -1
integration_time = global_header["integration_time"]
nchan = global_header["number_channels"]
timeslice_bin = corfile.read(size_timeslice)
while timeslice_bin != "":
  slicenr, nbaseline, nuvw, nstat = struct.unpack('4i', timeslice_bin)
  if (current_scan < len(timeslices) - 1) and \
     (slicenr >= timeslices[current_scan + 1][0]):
    current_scan += 1
    splines, pointing_centre = get_splines(delays, stations, timeslices[current_scan][1])
    frequencies = get_frequencies(vex, setup_station, timeslices[current_scan][1])
    # DEBUG print
    #for source in splines:
    #  for stationnr in splines[source]:
    #    station = stations[stationnr]
    #    for i in range(int((timeslices[current_scan+1][0]-timeslices[current_scan][0]-1))):
    #      t = timeslices[current_scan][1] + (i+0.5) * integration_time*1e-6;
    #      print 'splines['+source+']['+station+']: t= %.15g, delay = %.15g'%(t, splines[source][stationnr]["delay"](t))

  # mid time of current slice
  dslice = (slicenr - timeslices[current_scan][0])
  tmid = timeslices[current_scan][1] + (dslice+0.5) * integration_time / 1e6
  
  # process uvw
  uvws = []
  for i in range(nuvw):
    uvws.append(struct.unpack('2i3d', corfile.read(size_uvw)))
  
  # sampler stats
  stats = []
  for i in range(nstat):
    stats.append(corfile.read(size_statistics))
  
  # read data
  baselines = []
  for bl in range(nbaseline):
    h_bin = corfile.read(size_baseline)
    h = struct.unpack('i4b', h_bin)
    data = fromfile(corfile, dtype='c8', count = (nchan + 1))
    baselines.append((h, h_bin, data))
  
  # perform the actual uv-shifting 
  # FIXME only works for one scan per file!
  for source in splines:
    if source == pointing_centre:
      continue
    try:
      outfile = outfiles[source]
    except KeyError:
      # FIXME check for file existance before overwriting!
      filename = corfile.name+'_'+source
      outfile = open(filename, 'w')
      outfiles[source] = outfile
      outfile.write(global_header_bin)
    # Time slice header
    outfile.write(timeslice_bin)
    # Process uvws
    for uvw in uvws:
      station = uvw[0]
      model = splines[source][station]
      u, v, w  = (model['u'](tmid), model['v'](tmid), model['w'](tmid))
      uvw_bin = struct.pack('2i3d', uvw[0], uvw[1], u, v, w)
      outfile.write(uvw_bin)
    # Sampler statistics (unmodified)
    for stat in stats:
      outfile.write(stat)
    # uvshift the baselines
    for (h, h_bin, data) in baselines:
      s1 = int(h[1])
      s2 = int(h[2])
      sb = int((h[3]>>2)&1)
      freq_nr = int((h[3]>>3)&31)
      freq, bw = frequencies[freq_nr][sb]
      # The delay offsets 
      delay1_0 = splines[pointing_centre][s1]["delay"](tmid)
      delay1_1 = splines[source][s1]["delay"](tmid)
      ddelay1 = delay1_1 - delay1_0
      delay2_0 = splines[pointing_centre][s2]["delay"](tmid)
      delay2_1 = splines[source][s2]["delay"](tmid)
      ddelay2 = delay2_1 - delay2_0
      rate1 = splines[source][s1]["rate"](tmid)
      rate2 = splines[source][s2]["rate"](tmid)
      # Compute phase offsets
      dfreq = (2*sb-1) * bw / (2*nchan) # Evaluate centre frequency of channel
      phi0 = (freq + dfreq) * (ddelay1 * (1 - rate1) - ddelay2 * (1 - rate2))
      phi0 = 2 * pi * (2*sb-1) * (phi0-floor(phi0))
      # assuming nyquist sampling!
      delta = 2 * pi * (ddelay1 * (1 - rate1) - ddelay2 * (1 - rate2)) * bw / nchan
      if (slicenr == -9) and (s1 != s2):
        pdb.set_trace()
      phi = phi0 + arange(nchan+1) * delta
      newdata = data * exp(1j*phi, dtype=complex64)
      # Write results
      outfile.write(h_bin)
      newdata.tofile(outfile)
  # Read next time slice
  timeslice_bin = corfile.read(size_timeslice)
