#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys, struct
from numpy import *
from pylab import *
from time import sleep
from optparse import OptionParser
import pdb

timeslice_header_size = 16
uvw_header_size = 32
stat_header_size = 24
baseline_header_size = 8

plots_per_row = 2   # The number of pulse profiles per row in the plot window

def read_scan(inputfiles, profile, station_idx, ref_station, nchan, scalar_avg):
  nbins = profile.shape[0]
  nstation = profile.shape[1]
  nif = profile.shape[2]
  nsb = profile.shape[3]
  npol = profile.shape[4]
  for bin in range(nbins):
    if scalar_avg:
      data = zeros([nstation, nif, nsb, npol])
    else:
      data = zeros([nstation, nif, nsb, npol], dtype='complex128')
    inputfile = inputfiles[bin]
    #get timeslice header
    tsheader_buf = inputfile.read(timeslice_header_size)
    while len(tsheader_buf) == timeslice_header_size:
      timeslice_header = struct.unpack('4i', tsheader_buf)
      nuvw = timeslice_header[2]
      inputfile.seek(uvw_header_size * nuvw, 1)
      # Read the bit statistics
      nstatistics = timeslice_header[3]
      stat_buffer = inputfile.seek(stat_header_size * nstatistics, 1)
      
      nbaseline = timeslice_header[1]
      baseline_data_size = (nchan + 1) * 8 # data is complex floats
      baseline_buffer = inputfile.read(nbaseline * (baseline_header_size + baseline_data_size)) 
      fmt = str(2*(nchan+1)) + 'f'

      index = 0
      for b in range(nbaseline):
        bheader = struct.unpack('i4c', baseline_buffer[index:index + baseline_header_size])
        index += baseline_header_size
        station1 = struct.unpack('i', bheader[1]+'\x00\x00\x00')[0]
        station2 = struct.unpack('i', bheader[2]+'\x00\x00\x00')[0]
        byte = struct.unpack('i', bheader[3]+'\x00\x00\x00')[0]
        pol1 = byte&1
        pol2 = (byte>>1)&1
        sideband = (byte>>2)&1
        freq_nr = byte>>3
        #print 'pol1='+str(pol1) +  ', pol2='+str(pol2)+',sideband='+str(sideband)+',freq_nr='+str(freq_nr)+' ; s1='+str(station1)+', s2='+str(station2)
        if ((station1 == ref_station and station2 != ref_station) or (station2 == ref_station and station1 != ref_station)) and (pol1 == pol2):
          if station1 == ref_station:
            station = station_idx[station2]
          else:
            station = station_idx[station1]
          if npol == 1:
            pol_idx = 0
          else:
            pol_idx = pol1
          if nsb == 1:
            sb_idx = 0
          else:
            sb_idx = sideband

          buf = struct.unpack(fmt,baseline_buffer[index:index + baseline_data_size])
          if scalar_avg:
            # Skip over the first/last channel
            vis = sum(square(buf[2:2*nchan]))
          else:
            # Skip over the first/last channel
            vreal = sum(buf[2:2*nchan:2])
            vim = sum(buf[3:2*nchan:2])
            vis = vreal + 1j * vim
          if isnan(abs(vis))==False:
            #pdb.set_trace()
            data[station, freq_nr, sb_idx, pol_idx] += vis
        index += baseline_data_size
      tsheader_buf = inputfile.read(timeslice_header_size)
    profile[bin,:] += abs(data)

def get_station_list(vexfile):
  stations = []
  try:
    vex = open(vexfile, 'r')
  except:
    print 'Error : could not open vexfile : ' + vexfile
    sys.exit(1)
  line = vex.readline()
  while (line != '') and (line.lstrip().startswith('$STATION') == False):
    line = vex.readline()
  if(line == ''):
    print 'Couldn\'t find station block in vexfile'
    sys.exit(1)

  line = vex.readline()
  while (line != '') and (line.lstrip().startswith('$') == False):
    line = line.lstrip()
    if(line.startswith('def')):
      end = line.find(';')
      if(end < 0):
	print 'Error parsing vex file, station definition doesn\'t end with ;'
        print '  : ' + line
        exit(1)
      station = line[4:end].lstrip()
      stations.append(station)
      print 'found station #%d : %s'%(len(stations), station)
    line = vex.readline()
  vex.close()
  stations.sort()
  return stations

def initialize(base_file_name, nbins, station_list):
  inputfiles = []
  # open all input files and read global header
  for bin in range(nbins):
    filename = base_file_name + '.bin'+str(bin)
    try:
      inputfile = open(filename, 'rb')
    except:
      print "Error : Could not open " + filename
      sys.exit(1)
    gheader_size_buf = inputfile.read(4)
    global_header_size = struct.unpack('i', gheader_size_buf)[0]
    inputfile.seek(0)
    gheader_buf = inputfile.read(global_header_size)
    global_header = struct.unpack('i32s2h5i4c',gheader_buf[:64])
    nchan = global_header[5]
    inputfiles.append(inputfile)
  # determine parameters from first bin
  inputfile = inputfiles[0]
  #get timeslice header
  tsheader_buf = inputfile.read(timeslice_header_size)
  timeslice_header = struct.unpack('4i', tsheader_buf)
  integration_slice = timeslice_header[0]
  stations_found = zeros(len(station_list))
  nsubint = 0
  pol=0
  sb=0
  nif=0
  while(integration_slice == 0):
    # get the uvw buffer
    nuvw = timeslice_header[2]
    inputfile.seek(uvw_header_size * nuvw, 1)
    
    # Read the bit statistics
    nstatistics = timeslice_header[3]
    stat_buffer = inputfile.seek(stat_header_size * nstatistics, 1)
    
    nbaseline = timeslice_header[1]
    baseline_data_size = (nchan + 1) * 8 # data is complex floats
    baseline_buffer = inputfile.read(nbaseline * (baseline_header_size + baseline_data_size)) 

    index = 0
    for b in range(nbaseline):
      bheader = struct.unpack('i4c', baseline_buffer[index:index + baseline_header_size])
      index += baseline_header_size
      station1 = struct.unpack('i', bheader[1]+'\x00\x00\x00')[0]
      station2 = struct.unpack('i', bheader[2]+'\x00\x00\x00')[0]
      stations_found[station1] = 1
      stations_found[station2] = 1
      byte = struct.unpack('i', bheader[3]+'\x00\x00\x00')[0]
      pol |=  byte&3
      sb |= ((byte>>2)&1) + 1 
      nif = max((byte>>3) + 1, nif)
      print 's1=%d, s2=%d, pol=%d, sb=%d, nif=%d, if_found=%d, sb_found=%d'%(station1, station2, pol, sb, nif, byte>>3, (byte>>2)&1)
      index += baseline_data_size
    tsheader_buf = inputfile.read(timeslice_header_size)
    timeslice_header = struct.unpack('4i', tsheader_buf)
    integration_slice = timeslice_header[0]
    nsubint += 1
  
  inputfile.seek(global_header_size)
  stations_in_job = []
  for i in range(stations_found.size):
    if stations_found[i] == 1:
      stations_in_job.append(i)
  nsb = 2 if sb ==3 else 1
  return (inputfiles, nchan, nif, nsb, pol, stations_in_job, nsubint)

def get_options():
  parser = OptionParser('%prog [options] <vex file> <base_file_name 1> ... <base_file_name N> <nbins> <ref_station>')
  parser.add_option("-s", "--scalar-avg", action="store_true", dest="scalar_avg", default = False,
                    help = "Scalar average within a scan [default = vector average], " +
                           " note that if there is more than one scan then the results of each scan " +
                           " are always scalar averaged.")
  (options, args) = parser.parse_args()
  N = len(args)
  if N < 4:
    parser.error('Invalid number of arguments')
  base_file_names = []
  for i in range(N-3):
    base_file_names.append(args[1+i])
  return(args[0], base_file_names, int(args[N-2]), args[N-1], options.scalar_avg)

def update_plots(profile, ax):
  nbins = profile.shape[0]
  nstation = profile.shape[1]
  nif = profile.shape[2]
  nsb = profile.shape[3]
  npol = profile.shape[4]
  bins = abs(profile).sum(2).sum(2).sum(2)
  interactive(True)
  #pdb.set_trace()
  for i in range(nstation):
    divider = max(bins[:, i].max(), 1)
    data = bins[:, i] / divider
    data = data - data.min()
    ax[i].plot(data)
    ax[i].set_xlim(0, nbins)
    ax[i].set_ylim(0, 1)
  draw()

def create_window(station_list, stations_in_job, ref_station, nbins):
  fig = figure(1, figsize=(8,6),dpi=100)
  fig.suptitle('Pulse profiles for station %s'%(station_list[ref_station]) , fontsize=12)
  nstation = len(stations_in_job)
  nrows = int(ceil(nstation*1./ plots_per_row))

  ax = []
  idx = 0
  xplots = plots_per_row 
  yplots = int(ceil((nstation-1) * 1. / xplots))
  for i in range(nstation):
    station = stations_in_job[i]
    if station != ref_station:
      idx += 1
      ax.append(fig.add_subplot(yplots, xplots, idx))
      ax[-1].set_title('%s - %s'%(station_list[station], station_list[ref_station]))
  return ax

######### MAIN CODE
(vexfile, base_file_names, nbins, ref_station_name, scalar_avg) = get_options()
station_list = get_station_list(vexfile)
try:
  ref_station = station_list.index(ref_station_name)
except:
  print 'Ref station %s is not in datafiles'%(ref_station_name)
  sys.exit(1)

# Parse the input files
scans = []
stations_in_job = set()
for base_file_name in base_file_names:
  scan = {}
  (inputfiles, nchan, nif, sb, pol, stations_in_scan, nsubint) = initialize(base_file_name, nbins, station_list)
  scan["files"] = inputfiles
  scan["nchan"] = nchan
  scans.append(scan)
  stations_in_job = stations_in_job.union(stations_in_scan)
stations_in_job = list(stations_in_job)

station_idx = zeros(max(stations_in_job)+1,dtype=int)
index = 0
for i in range(len(stations_in_job)):
  if stations_in_job[i] != ref_station:
    station_idx[stations_in_job[i]] = index
    index += 1
nstation = len(stations_in_job)
npol = 2 if (pol == 3) else 1

profile = zeros([nbins, nstation - 1, nif, sb, npol])
ax = create_window(station_list, stations_in_job, ref_station, nbins)
for scan in scans:
  read_scan(scan["files"], profile, station_idx, ref_station, scan["nchan"], scalar_avg)
  update_plots(profile, ax)
show()

