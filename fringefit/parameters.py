#!/usr/bin/env python
import sys, struct, datetime, pdb
import vex_parser, vex_time
from sfxcdata_utils import *

class parameters:
  def __init__(self, vex, corfilename, timeout = 0):
    self.vex = vex
    self.timeout = timeout
    try:
      inputfile = open(corfilename, 'rb')
    except:
      print >> sys.stderr, "Error : Could not open " + corfilename
      sys.exit(1)
    
    gheader_size_buf = read_data(inputfile, 4, timeout)
    self.global_header_size = struct.unpack('i', gheader_size_buf)[0]
    inputfile.seek(0)
    gheader_buf = read_data(inputfile, self.global_header_size, timeout)
    global_header = struct.unpack('i32s2h5i4c',gheader_buf[:64])
    self.nchan = global_header[5]
    self.integration_time = global_header[6]/1000000.
    # get timeslice header
    tsheader_buf = read_data(inputfile, timeslice_header_size, timeout)
    timeslice_header = struct.unpack('4i', tsheader_buf)
    integration_slice = timeslice_header[0]
    nsubint = 0
    self.starttime = vex_time.get_date_string(global_header[2], global_header[3], global_header[4])
    scan = self.get_scan(self.starttime)
    self.get_channels(scan["mode"])
    stations_found = [False] *len(vex['STATION'])
    channels_found = [False] * len(self.vex_channels)
    size_of_slice = 0
     
    while(integration_slice == 0):
      # get the uvw buffer
      nuvw = timeslice_header[2]
      read_data(inputfile, uvw_header_size * nuvw, timeout)
      size_of_slice += uvw_header_size * nuvw
      # Read the bit statistics
      nstatistics = timeslice_header[3]
      read_data(inputfile, stat_header_size * nstatistics, timeout)
      size_of_slice += stat_header_size * nstatistics
      
      nbaseline = timeslice_header[1]
      baseline_data_size = (self.nchan + 1) * 8 # data is complex floats
      baseline_buffer = read_data(inputfile, nbaseline * (baseline_header_size + baseline_data_size), timeout) 
      size_of_slice += nbaseline * (baseline_header_size + baseline_data_size)
      
      index = 0
      baselines = []
      for b in range(nbaseline):
        bheader = struct.unpack('i4b', baseline_buffer[index:index + baseline_header_size])
        index += baseline_header_size
        station1 = bheader[1] 
        station2 = bheader[2]
        if not [station1,station2] in baselines:
          baselines.append([station1,station2])
        #print 's1 = ' + str(station1) + ', s2 = ' + str(station2)
        stations_found[station1] = True
        stations_found[station2] = True
        pol = bheader[3]&1
        sb = (bheader[3]&4)>>2
        freq = bheader[3] >> 3
        channels_found[self.vex_channels.index([freq,sb,pol])] = True
        index += baseline_data_size
      tsheader_buf = read_data(inputfile, timeslice_header_size, timeout)
      size_of_slice += timeslice_header_size
      timeslice_header = struct.unpack('4i', tsheader_buf)
      integration_slice = timeslice_header[0]
      nsubint += 1
    self.baselines = baselines

    # Generate stations list
    self.stations = []
    for i in range(len(stations_found)):
      if stations_found[i]:
        self.stations.append(i)
    # Generate list of channels
    self.channels = []
    for i in range(len(channels_found)):
      if channels_found[i]:
        self.channels.append(self.vex_channels[i])
    # number of integrations in file
    inputfile.seek(0,2)  
    filesize = inputfile.tell()
    self.n_integrations = int((filesize - self.global_header_size) / size_of_slice)
    inputfile.close()
    self.nsubint = nsubint

  def get_scan(self, t):
    vex = self.vex
    time = vex_time.get_time(t)
    for scan in vex['SCHED'].itervalues():
      scan_start = vex_time.get_time(scan["start"])
      duration = int(scan["station"][2].partition('sec')[0])
      scan_end = scan_start + datetime.timedelta(seconds = duration)
      if (time >= scan_start) and (time < scan_end):
        return scan

  def get_channels(self, mode):
    vex = self.vex
    ch_mode = vex['MODE'][mode]['FREQ'][0]
    station = vex['MODE'][mode]['FREQ'][1]
    for bmode in  vex['MODE'][mode].getall('BBC'):
      if station in bmode:
        bbc_mode = bmode[0]
    for imode in vex['MODE'][mode].getall('IF'):
      if station in imode:
        if_mode = imode[0]
    chandefs = vex['FREQ'][ch_mode].getall('chan_def')
    bbcs = vex['BBC'][bbc_mode].getall('BBC_assign')
    ifs = vex['IF'][if_mode].getall('if_def')
    self.sample_rate = float(vex['FREQ'][ch_mode]['sample_rate'].partition('Ms')[0])

    freqs = []
    channels = []
    channel_names = []
    for ch in chandefs:
      f = float(ch[1].strip('MHz'))
      try:
        freq = freqs.index(f)
      except:
        freq = len(freqs)
        freqs.append(f)
      ifreq = [b for b in bbcs if b[0] == ch[5]][0][2]
      polstring = [i for i in ifs if i[0] == ifreq][0][2]
      pol = 0 if polstring == 'R' else 1
      sb = 0 if ch[2] == 'L' else 1
      channels.append([freq,sb,pol])
      channel_names.append(ch[4])
    self.freqs = freqs
    self.vex_channels = channels
    self.channel_names = channel_names
