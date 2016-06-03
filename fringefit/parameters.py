#!/usr/bin/env python
import sys, struct
from datetime import datetime, timedelta
import vex_time
from sfxcdata_utils import *

class parameters:
  def __init__(self, vex, corfilename, setup_station, timeout = 0):
    self.vex = vex
    self.timeout = timeout
    self.setup_station = setup_station
    try:
      self.inputfile = open(corfilename, 'rb')
    except:
      print >> sys.stderr, "Error : Could not open " + corfilename
      sys.exit(1)
    
    gheader_size_buf = read_data(self.inputfile, 4, timeout)
    self.global_header_size = struct.unpack('i', gheader_size_buf)[0]
    self.inputfile.seek(0)
    gheader_buf = read_data(self.inputfile, self.global_header_size, timeout)
    global_header = struct.unpack('i32s2h5i4c',gheader_buf[:64])
    self.nchan = global_header[5]
    self.integration_time = global_header[6]/1000000.
    first_day_of_year = datetime(global_header[2], 1, 1)
    self.starttime = first_day_of_year + timedelta(days=global_header[3]-1, seconds=global_header[4])
    scan = self.get_scan_name(self.starttime)
    self.get_channels(vex['SCHED'][scan]["mode"])
    self.get_vex_scans()
    # Get list of scans in correlation file 
    self.scanlist = [self.read_scan_params(self.global_header_size)]
    self.scanlist[0]["name"] = scan
    self.scanlist[0]["start"] = self.starttime
    self.update_scanlist()

  def get_scan_name(self, time):
    vex = self.vex
    for scan_name in vex['SCHED']:
      scan = vex['SCHED'][scan_name]
      scan_start = vex_time.get_time(scan["start"])
      duration = self.get_scan_duration(scan_name)
      scan_end = scan_start + timedelta(seconds = duration)
      if (time >= scan_start) and (time < scan_end):
        return scan_name

  def get_scan_duration(self, scan_name):
    max_duration = 0
    for station in self.vex['SCHED'][scan_name].getall('station'):
      max_duration = max(max_duration, int(station[2].partition('sec')[0]))
    return max_duration
  
  def get_channels(self, mode):
    vex = self.vex
    setup_station = self.setup_station
    freqs = vex['MODE'][mode].getall('FREQ')
    ch_mode = ""
    for block in freqs:
      if setup_station in block:
        ch_mode = block[0]
    if ch_mode == "":
      print >> sys.stderr, "Error : Could not find FREQ block for setup station (" + setup_station + ") in mode " + mode
      sys.exit(1)
    for bmode in  vex['MODE'][mode].getall('BBC'):
      if setup_station in bmode:
        bbc_mode = bmode[0]
    for imode in vex['MODE'][mode].getall('IF'):
      if setup_station in imode:
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

  def get_vex_scans(self):
    vex_scans = []
    for v in self.vex['SCHED']:
      vex_scans.append((self.vex['SCHED'][v]["start"], v))
    vex_scans.sort()
    self.vex_scans = vex_scans

  def update_scanlist(self):
    self.inputfile.seek(0,2)
    filesize = self.inputfile.tell()
    vex = self.vex
    vex_scans = self.vex_scans
    scanlist = self.scanlist
    scanname = scanlist[-1]["name"]
    scannr = 0
    while vex_scans[scannr][1] != scanname:
      scannr += 1
    while True:
      scanstart = vex_time.get_time(vex["SCHED"][scanname]["start"])
      duration = self.get_scan_duration(scanname)
      dt = scanlist[-1]["start"] - scanstart
      duration -= dt.seconds
      nint = int(duration/self.integration_time)
      endpos = scanlist[-1]["fpos"] + nint*scanlist[-1]["slice_size"]
      if (endpos > filesize) or (scannr == len(vex_scans)-1):
        break
      try:
        newscan = self.read_scan_params(endpos)
        scanlist.append(newscan)
      except EndOfData:
        break
      scannr += 1
      scanstart = vex_time.get_time(vex_scans[scannr][0])
      scanname = vex_scans[scannr][1]
      scanlist[-1]["name"] = scanname
      scanlist[-1]["start"] = scanstart 

  def get_scan_param(self, scanname):
    self.update_scanlist()
    for scan in self.scanlist:
      if scan["name"] == scanname:
        return scan
    return None

  def read_scan_params(self, filepos):
    vex = self.vex
    timeout = self.timeout
    inputfile = self.inputfile
    inputfile.seek(filepos)
    stations_found = [False] *len(vex['STATION'])
    channels_found = [False] * len(self.vex_channels)
    # get timeslice header
    tsheader_buf = read_data(inputfile, timeslice_header_size, timeout)
    timeslice_header = struct.unpack('4i', tsheader_buf)
    start_slice = timeslice_header[0]
    integration_slice = start_slice
    size_of_slice = 0
    nsubint = 0
    while(integration_slice == start_slice):
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
      for b in range(nbaseline):
        bheader = struct.unpack('i4b', baseline_buffer[index:index + baseline_header_size])
        index += baseline_header_size
        station1 = bheader[1] 
        station2 = bheader[2]
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
    # Generate stations list
    stations = []
    for i in range(len(stations_found)):
      if stations_found[i]:
        stations.append(i)
    # Generate list of channels
    channels = []
    for i in range(len(channels_found)):
      if channels_found[i]:
        channels.append(self.vex_channels[i])
    scan = {"start_slice": start_slice, "fpos": filepos,\
            "slice_size":  size_of_slice, "nsubint": nsubint,\
            "stations": stations, "channels": channels}
    return scan 
