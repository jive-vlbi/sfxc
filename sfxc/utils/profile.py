#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys, struct
from numpy import *
from pylab import *
from time import sleep
from optparse import OptionParser
import pdb

global_header_size = 64
timeslice_header_size = 16
uvw_header_size = 32
stat_header_size = 24
baseline_header_size = 8

plots_per_row = 2   # The number of pulse profiles per row in the plot window

def read_time_slice(inputfiles, visibilities, station_idx, ref_station, pol, sb, nchan):
  nbins = len(inputfiles)
  nif = visibilities.shape[2]
  num_sb = visibilities.shape[3]
  npol = visibilities.shape[4]
  for bin in range(nbins):
    inputfile = inputfiles[bin]
    #get timeslice header
    tsheader_buf = read_data(inputfile, timeslice_header_size)
    timeslice_header = struct.unpack('4i', tsheader_buf)
    
    nuvw = timeslice_header[2]
    inputfile.seek(uvw_header_size * nuvw, 1)
    
    # Read the bit statistics
    nstatistics = timeslice_header[3]
    stat_buffer = inputfile.seek(stat_header_size * nstatistics, 1)
    
    nbaseline = timeslice_header[1]
    baseline_data_size = (nchan + 1) * 8 # data is complex floats
    baseline_buffer = read_data(inputfile, nbaseline * (baseline_header_size + baseline_data_size)) 
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
      J = complex(0,1)
      if ((station1 == ref_station and station2 != ref_station) or (station2 == ref_station and station1 != ref_station)) and (pol1 == pol2):
        if station1 == ref_station:
          station = station_idx[station2]
        else:
          station = station_idx[station1]
        if npol == 1:
	  pol_idx = 0
	else:
	  pol_idx = pol1
        if num_sb == 1:
	  sb_idx = 0
	else:
	  sb_idx = sideband

        buf = struct.unpack(fmt,baseline_buffer[index:index + baseline_data_size])
	# Skip over the first/last channel
        vreal = sum(buf[2:2*nchan:2])
        vim = sum(buf[3:2*nchan:2])
	if isnan(vreal)==False and isnan(vim)==False:
          #pdb.set_trace()
	  visibilities[station, bin, freq_nr, sb_idx, pol_idx] += vreal + J * vim
      index += baseline_data_size
  return visibilities

def read_data(inputfile, ndata):
  data = ''
  data_read = 0
  while data_read < ndata:
    data += inputfile.read(ndata - data_read)
    data_read = len(data)
    if data_read < ndata:
      sleep(1) # if not enough data is available sleep for 1 sec
  return data

def update_plots(visibilities, plots):
  nstation = visibilities.shape[0]
  nbins = visibilities.shape[1]
  nif = visibilities.shape[2]
  nside = visibilities.shape[3]
  bins = abs(visibilities).sum(2).sum(2).sum(2)
  interactive(True)
  #pdb.set_trace()
  for i in range(nstation):
    divider = max(bins[i,:].max(), 1)
    data = bins[i,:] / divider
    data = data - data.min()
    plots[i].set_ydata(data)
  draw()

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
    gheader_buf = read_data(inputfile, global_header_size)
    global_header = struct.unpack('i32s2h5i4c',gheader_buf)
    nchan = global_header[5]
    inputfiles.append(inputfile)
  # determine parameters from first bin
  inputfile = inputfiles[0]
  #get timeslice header
  tsheader_buf = read_data(inputfile, timeslice_header_size)
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
    baseline_buffer = read_data(inputfile, nbaseline * (baseline_header_size + baseline_data_size)) 

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
    tsheader_buf = read_data(inputfile, timeslice_header_size)
    timeslice_header = struct.unpack('4i', tsheader_buf)
    integration_slice = timeslice_header[0]
    nsubint += 1
  
  inputfile.seek(64)
  stations_in_job = []
  for i in range(stations_found.size):
    if stations_found[i] == 1:
      stations_in_job.append(i)
  return (inputfiles, nchan, nif, sb, pol, stations_in_job, nsubint)

def create_plot_window(station_list, stations_in_job, ref_station, nbins):
  plots = []
  interactive(True)
  fig = figure()
  suptitle('Pulse profiles for station %s'%(station_list[ref_station]) , fontsize=12)
  nstation = len(stations_in_job) - 1
  nrows = int(ceil(nstation*1./ plots_per_row))
  f = 1
  empty_data = zeros([nbins])

  for i in range(len(stations_in_job)):
    station = stations_in_job[i]
    if  station != ref_station:
      plt = nrows * 100 + plots_per_row * 10 + f
      subplot(plt)
      p, = plot(empty_data)
      title('%s - %s'%(station_list[station], station_list[ref_station]))
      axis([0, nbins, 0, 1])
      draw()
      plots.append(p)
      f += 1
  return plots

def get_options():
  parser = OptionParser('%prog <vex file> <base_file_name> <nbins> <ref_station>')
  (options, args) = parser.parse_args()
  if len(args) != 4:
    parser.error('Invalid number of arguments')
  return(args[0], args[1], int(args[2]), args[3])

######### MAIN CODE
(vexfile, base_file_name, nbins, ref_station_name) = get_options()
station_list = get_station_list(vexfile)
try:
  ref_station = station_list.index(ref_station_name)
except:
  print 'Ref station %s is not in datafiles'%(ref_station_name)
  sys.exit(1)
(inputfiles, nchan, nif, sb, pol, stations_in_job, nsubint) = initialize(base_file_name, nbins, station_list)

station_idx = zeros(max(stations_in_job)+1,dtype=int)
#pdb.set_trace()
index = 0
for i in range(len(stations_in_job)):
  if stations_in_job[i] != ref_station:
    station_idx[stations_in_job[i]] = index
    index += 1

nstation = len(stations_in_job)
num_sb = (sb&1) + (sb>>1)
if pol == 3:
  npol = 2
else:
  npol = 1
visibilities = zeros([nstation - 1, nbins, nif, num_sb, npol], dtype=complex128)
plots = create_plot_window(station_list, stations_in_job, ref_station, nbins)
time_slice = 0
while True:
  #read one integration
  read_time_slice(inputfiles, visibilities, station_idx, ref_station, pol, sb, nchan)
  time_slice += 1
  #update plot
  if time_slice % nsubint == 0:
    update_plots(visibilities, plots)
