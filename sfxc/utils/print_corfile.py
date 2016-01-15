#!/usr/bin/env python
import sys, struct, pdb
from numpy import *
from optparse import OptionParser

timeslice_header_size = 16
uvw_header_size = 32
stat_header_size = 24
baseline_header_size = 8
max_stations = 32
fringe_guard = 0.05  # Used to compute the SNR, this is the percentage that is ignored around the maximum

def print_global_header(infile):
  infile.seek(0)
  gheader_buf = infile.read(global_header_size)
  if global_header_size == 64:
    global_header = struct.unpack('i32s2h5ib3c',gheader_buf[:64])
  else:
    global_header = struct.unpack('i32s2h5ib15s',gheader_buf[:76])
  hour = global_header[4] / (60*60)
  minute = (global_header[4]%(60*60))/60
  second = global_header[4]%60
  pol = ['LL', 'RR', 'LL+RR', 'LL+RR+LR+RL'][global_header[9]]
  
  if global_header_size == 64:
    print "SFXC version = %s"%(global_header[8])
  else:
    n = global_header[-1].index('\0')
    print "SFXC version = %s, branch = %s"%(global_header[8], global_header[-1][:n])

  n = global_header[1].index('\0')
  print "Experiment %s, date = %dy%dd%dh%dm%ds, int_time = %d, nchan = %d, polarization = %s"%(global_header[1][:n], global_header[2], global_header[3], hour, minute, second, global_header[6], global_header[5], pol)

def print_uvw(uvws, stations=None):
  for uvw in uvws.iteritems():
    if stations == None:
      print "Station ", uvw[0]
    else:
      print "Station ", stations[uvw[0]]
    for nstr in uvw[1]:
      print nstr

def print_stats(stats, stations=None):
  for stat in stats.iteritems():
    if stations == None:
      print "Station ", stat[0]
    else:
      print "Station ", stations[stat[0]]
    for nstr in stat[1]:
      print nstr

def print_baselines(data, stations=None):
  keys = data.keys()
  keys.sort()
  for key in keys:
    bl = data[key]
    if stations == None:
      print "Baseline :  station1 = ", key[0], ", station2 = ", key[1]
    else:
      print "Baseline :  station1 = ", stations[key[0]], ", station2 = ", stations[key[1]]
    for nstr in bl:
      print nstr

def get_baseline_stats(data):
  data[0] = data[0].real
  real_data = abs(fft.fftshift(fft.irfft(data)))
  n = real_data.size
  fringe_pos = real_data.argmax()
  fringe_val = real_data[fringe_pos]
  guard = max(round(real_data.size * fringe_guard), 1)
  avg  = real_data[0:max(fringe_pos-guard, 0)].sum()
  avg += real_data[min(fringe_pos+guard, n):n].sum()
  navg = n - (fringe_pos - max(fringe_pos-guard, 0)) - (min(fringe_pos+guard, n) - fringe_pos)
  avg /= max(navg, 1)
  noise  = pow(real_data[0:max(fringe_pos-guard, 0)] - avg, 2).sum()
  noise += pow(real_data[min(fringe_pos+guard, n):n] - avg, 2).sum()
  snr = sqrt(((fringe_val - avg)**2) * navg / max(noise,1e-12))
  fringe_offset = int(round((fringe_pos - n / 2) / 2.)) # devide by two because of oversampling
  return (fringe_val, snr, fringe_offset)

def read_uvw(infile, uvw, nuvw):
  uvw_buffer = infile.read(uvw_header_size * nuvw)
  if len(uvw_buffer) != uvw_header_size * nuvw:
    raise Exception("EOF")
  index = 0
  for i in range(nuvw):
    header = struct.unpack('2i3d', uvw_buffer[index:index + uvw_header_size])
    index += uvw_header_size
    station_nr = header[0]
    (u,v,w) = header[2:]
    nstr = 'u = %.15g, v = %.15g, w = %.15g'%(u,v,w)
    try:
      uvw[station_nr].append(nstr)
    except KeyError:
      uvw[station_nr] = [nstr]

def read_statistics(infile, stats, nstatistics):
  stat_buffer = infile.read(stat_header_size * nstatistics)
  if len(stat_buffer) != stat_header_size * nstatistics:
    raise Exception("EOF")
  index = 0
  for i in range(nstatistics):
    sheader = struct.unpack('4B5i', stat_buffer[index:index + stat_header_size])
    index += stat_header_size
    station_nr = sheader[0]
    frequency_nr = sheader[1]
    sideband = sheader[2]
    polarisation = sheader[3]
    levels = sheader[4:9]
    tot = sum(levels) + 0.0001
    levels = [a / tot for a in levels]
    nstr = 'freq = ' + str(frequency_nr) + ", sb = " + str(sideband) + ", pol = " + str(polarisation) + \
           ", levels : --=" + str(levels[0]) + " -+ = " + str(levels[1]) +  " +- = " + str(levels[2]) +  " ++= " + str(levels[3]) + \
           " invalid = " + str(levels[4])
    try:
      stats[station_nr].append(nstr)
    except KeyError:
      stats[station_nr] = [nstr]

def read_baselines(infile, data, nbaseline, nchan):
  baseline_data_size = (nchan + 1) * 8 # data is complex floats
  baseline_buffer = infile.read(nbaseline * (baseline_header_size + baseline_data_size)) 
  if len(baseline_buffer) != nbaseline * (baseline_header_size + baseline_data_size):
    raise Exception("EOF")
  fmt = str(2*(nchan+1)) + 'f'

  index = 0
  for b in range(nbaseline):
    bheader = struct.unpack('i4B', baseline_buffer[index:index + baseline_header_size])
    index += baseline_header_size
    weight = bheader[0]
    station1 = bheader[1]
    station2 = bheader[2]
    baseline = (station1, station2)
    byte = bheader[3]
    pol = byte&3
    sideband = (byte>>2)&1
    freq_nr = byte>>3
    J = complex(0,1)
    if station1 != station2:
      buf = struct.unpack(fmt, baseline_buffer[index:index + baseline_data_size])
      # Skip over the first/last channel
      vreal = array(buf[0:2*(nchan+1):2])
      vim = array(buf[1:2*(nchan+1):2])
      if isnan(vreal).any()==False and isnan(vim).any()==False:
        #pdb.set_trace()
        # format is [nbaseline, nif, num_sb, npol, nchan+1], dtype=complex128
        val, snr, offset = get_baseline_stats(vreal + J*vim)
        nstr = 'freq = %d, sb = %d , pol = %d, fringe ampl = %.6f , SNR = %.6f, offset = %d, weight = %.6f'%(freq_nr, sideband, pol, val, snr, offset, weight)
        try:
          data[baseline].append(nstr)
        except KeyError:
          data[baseline] = [nstr]
      else:
        print "b="+`baseline`+", freq_nr = "+`freq_nr`+",sb="+`sideband`+",pol="+`pol`
        pdb.set_trace()
    index += baseline_data_size

def read_time_slice(infile, stats, uvw, data, nchan):
  #get timeslice header
  tsheader_buf = infile.read(timeslice_header_size)
  if len(tsheader_buf) != timeslice_header_size:
    raise Exception("EOF")
  timeslice_header = struct.unpack('4i', tsheader_buf)
  current_slice = timeslice_header[0]  
  
  while current_slice == timeslice_header[0]:
    #Read UVW coordinates
    nuvw = timeslice_header[2]
    read_uvw(infile, uvw, nuvw)
    # Read the bit statistics
    nstatistics = timeslice_header[3]
    read_statistics(infile, stats, nstatistics)
    # Read the baseline data    
    nbaseline = timeslice_header[1]
    read_baselines(infile, data, nbaseline, nchan)
    # Get next time slice header
    tsheader_buf = infile.read(timeslice_header_size)
    if len(tsheader_buf) != timeslice_header_size:
      return 
    timeslice_header = struct.unpack('4i', tsheader_buf)
  # We read one time slice to many
  infile.seek(-timeslice_header_size, 1) 

def get_stations(vex_file):
  f = open(vex_file, 'r')
  stations = []
  rawline = f.readline()
  line = rawline.lstrip().upper()
  while rawline != "" and line[:9] != "$STATION;":
    rawline = f.readline()
    line = rawline.lstrip().upper()
  # Now get the station names
  rawline = f.readline()
  line = rawline.lstrip()
  while rawline != "" and line[:1] != "$":
    if line[:4] == "def ":
      z = line.find(';')
      stations.append(line[4:z].lstrip())
    rawline = f.readline()
    line = rawline.lstrip()

  if len(stations) == 0:
    print "Error, no stations found in vex_file : ", vex_file
    sys.exit(1)
  stations.sort()
  return stations

def get_options():
  parser = OptionParser('%prog [options] <correlator output file>')
  parser.add_option('-S', '--no-sampler-stats', action='store_false', dest="printstats", 
                    default=True, help='Do not print sampler statistics')
  parser.add_option('-V', '--no-visibilities', action='store_false', dest="printvis", 
                    default=True, help='Do not print visibilities')
  parser.add_option('-U', '--no-uvw', action='store_false', dest="printuvw", 
                    default=True, help='Do not print UVW coordinates')
  parser.add_option('-n', '--no-header', action='store_true',
                    default=False, help='Do not print the global header')
  parser.add_option('-v', '--vex', dest="vex_file", type="string",
                    help='Get station names from vex file (has to be the same as used during correlation)')
  (opts, args) = parser.parse_args()
  if len(args) == 0:
    parser.print_help()
    parser.exit()
  if len(args) != 1:
    parser.error('No correlator file specified')
  return (args[0], opts.no_header, opts.printstats, opts.printvis, opts.printuvw, opts.vex_file)

############################## Main program ##########################

filename, noheader, printstats, printvis, printuvw, vex_file = get_options()

try:
  infile = open(filename, 'rb')
except:
  print "Could not open file : " + filename
  sys.exit()

# Get list of station names
stations = get_stations(vex_file) if vex_file != None else None

# Read global header
global_header_size = struct.unpack('i', infile.read(4))[0]
if not noheader:
  print_global_header(infile)

infile.seek(0)
gheader_buf = infile.read(global_header_size)
global_header = struct.unpack('i32s2h5i4c',gheader_buf[:64])
nchan = global_header[5]
nslices = 0
while True:
  stats = {}
  uvw = {}
  data = {}
  try:
    read_time_slice(infile, stats, uvw, data, nchan)
    nslices += 1
  except Exception, e:
    if e.args[0] != 'EOF':
      raise
    print "Reached end of file"
    sys.exit(0)
   
  print "---------- time slice ", nslices," ---------"
  if printstats:
    print_stats(stats, stations)

  if printuvw:
    print_uvw(uvw, stations)

  if printvis:
    print_baselines(data, stations)
