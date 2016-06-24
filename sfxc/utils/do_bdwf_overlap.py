#!/usr/bin/env python
from vex import Vex
from optparse import OptionParser
from numpy import *
import struct, time, os
import pdb

# Size of headers in correlation file
size_timeslice = 16 # 4 integers
size_uvw = 32 # 2 integers and 3 doubles
size_baseline = 8 # 1 integer and 4 char (one char padding)
size_statistics = 24 # 4 chars, 5 integers

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

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
  usage = '%prog <vex file> <correlation file> <output file> <overlap factor>'
  parser = OptionParser(usage=usage)
  parser.add_option('-o', '--overlap-apply', dest='overlap_apply',
                    help='Overlap factor to apply during overlapping',
                    type='int')
  opts, args = parser.parse_args()
  if len(args) != 4:
    parser.error('Incorrect number of arguments')

  vex = Vex(args[0])
  corfile = open(args[1], 'r')
  if os.path.exists(args[2]):
    if not os.path.isfile(args[2]):
      parser.error(args[2]+' is not a file')
    else:
      while True:
        result = raw_input(args[2] + ' already exists, are you ' +\
                           'sure you want to overwrite it (yes/NO) : ')
        if result.lower() in ['', 'n', 'no']:
          exit(0)
        elif result.lower() == 'yes':
          break
  outfile = open(args[2], 'w')

  overlap = int(args[3])
  if (overlap < 0):
    parser.error('Overlap factor must be >= 0')
  if opts.overlap_apply == None:
    opts.overlap_apply = overlap
  elif (opts.overlap_apply < 0) or (opts.overlap_apply > overlap):
    parser.error('Invalid argument to overlap-apply')
  return vex, corfile, outfile, overlap, opts.overlap_apply

def new_integration():
  return {"data": [], "blheader": [], "tslice": [], "stats": [], "uvw": []}

def write_headers(outfile, h_tslice, h_uvw, h_stats, nbaseline):
  outfile.write(h_tslice[:4] + struct.pack("i", nbaseline) + h_tslice[8:])
  for h in h_uvw:
    outfile.write(h)
  for h in h_stats:
    outfile.write(h)

def write_baseline(outfile, bldata, blweight, blheaders):
  h = struct.pack('i4B', blweight, *blheaders[1:])
  outfile.write(h)
  bldata.tofile(outfile)

def do_overlap(outfile, integrations, nbaseline, nbdwf):
  n = len(integrations)
  m = n / 2
  
  for subint in range(len(integrations[m]["data"])):
    data = [integrations[z]["data"][subint] for z in range(n)]
    headers = [integrations[z]["blheader"][subint] for z in range(n)]
    # TODO stats should be modified for the overlapping
    write_headers(outfile, integration["tslice"][subint], integration["uvw"][subint], integration["stats"][subint], nbaseline)
    for bl in range(nbaseline):
      avdata = zeros((nchan+1), dtype='c8')
      # Do the overlap 
      blweight = 0
      s1, s2 =  headers[m][bl][1:3]
      for i in range(n):
        j = (i+1)/2 * (1 - 2*((i+1)%2)) # relative integration index
        avdata += data[m + j][bl + i*nbaseline]
        blweight += headers[m+j][bl + i*nbaseline][0]
      write_baseline(outfile, avdata, blweight, headers[m][bl])


#################################
#### MAIN
#################################

# Proper time.
os.environ['TZ'] = "UTC"

vex, corfile, outfile, overlap, overlap_apply = get_args()

global_header, global_header_bin = read_global_header(corfile)
outfile.write(global_header_bin)
timeslices = scan2timeslice(vex, global_header)

current_scan = -1
nbdwf = 2 * overlap + 1
napply = 2 * overlap_apply + 1

integration_time = global_header["integration_time"]
nchan = global_header["number_channels"]
timeslice_bin = corfile.read(size_timeslice)
old_slicenr = struct.unpack('i', timeslice_bin[:4])
integrations = []
while timeslice_bin != "":
  slicenr, nbaseline, nuvw, nstat = struct.unpack('4i', timeslice_bin)

  # Check if we have reached the next scan
  if (current_scan < len(timeslices) - 1) and \
     (slicenr >= timeslices[current_scan + 1][0]):
    # Write last integrations of previous scan
    for i in range(0, len(integrations), 2):
      do_overlap(outfile, integrations, nbaseline/nbdwf, nbdwf)
      integrations = integrations[2:]

    current_scan += 1
    integrations = [new_integration()]
    old_slicenr = slicenr
  # Check if we finished reading an integration
  if slicenr != old_slicenr:
    # NB: When less than 'napply' integrations have been read we overlap fewer times
    if (len(integrations)&1) == 1:
      do_overlap(outfile, integrations, nbaseline/nbdwf, nbdwf)
    # Only keep as many integrations as is needed for the overlapping
    if len(integrations) == napply:
      integrations = integrations[1:]
    integrations.append(new_integration())
    old_slicenr = slicenr
  integration = integrations[-1]
  integration["tslice"].append(timeslice_bin)
  # process uvw
  uvws = []
  for i in range(nuvw):
    uvws.append(corfile.read(size_uvw))
  integration["uvw"].append(uvws)
   
  # sampler stats
  stats = []
  for i in range(nstat):
    stats.append(corfile.read(size_statistics))
  integration["stats"].append(stats)
  
  # read data
  bldata = []
  blheader = []
  for bl in range(nbaseline):
    h = struct.unpack('i4B', corfile.read(size_baseline))
    data = fromfile(corfile, dtype='c8', count = (nchan + 1))
    blheader.append(h)
    bldata.append(data)
  integration["data"].append(bldata)
  integration["blheader"].append(blheader)
  # Read next time slice
  timeslice_bin = corfile.read(size_timeslice)

# Write the last integrations
for i in range(0, len(integrations), 2):
  do_overlap(outfile, integrations, nbaseline/nbdwf, nbdwf)
  integrations = integrations[2:]
