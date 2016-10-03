#!/usr/bin/env python
import sys, struct, pdb
import optparse
import datetime
import vex
import re
from numpy import *

timeslice_header_size = 16
uvw_header_size = 32
stat_header_size = 24
baseline_header_size = 8
nskip = 0
OLDBYTE = 300

def get_configuration(vexfile, corfile, setup_station):
  # TODO: This should also determine which subbands have been correlated
  corfile.seek(0)
  gheader_buf = corfile.read(global_header_size)
  global_header = struct.unpack('i32s2h5ib15s',gheader_buf[:76])
  cfg = {}
  cfg["nchan"] = global_header[5]
  cfg["inttime"] = global_header[6] / 1000000.
  cfg["npol"] = 1 if (global_header[9] < 2) else 2
  #FIXME This should include nskip
  cfg["start_time"] = get_time(global_header[2], global_header[3], global_header[4])

  tsheader_buf = corfile.read(timeslice_header_size)
  timeslice_header = struct.unpack('4i', tsheader_buf)
  nsubint = timeslice_header[1]
  cfg["nsubint"] = nsubint
  print global_header[6]/nsubint, nsubint
  if nsubint * (global_header[6]/nsubint) !=  global_header[6]:
    print "Error: There should be an integer number of subintegrations per integration"
    sys.exit(1)

  nsubband, maxfreq, bw = get_freq(vexfile, cfg["start_time"], setup_station)
  cfg["nsubband"] = nsubband
  cfg["maxfreq"] = maxfreq
  cfg["bandwidth"] = bw
  src_name, src_raj, src_dec = get_source(vexfile, cfg["start_time"])
  cfg["src_name"] = src_name
  cfg["src_raj"] = src_raj
  cfg["src_dec"] = src_dec

  cfg["mjd"] = mjd(global_header[2], global_header[3], global_header[4])
  print "jday=", cfg["mjd"], ", year = ", global_header[2], ", day=", global_header[3], ", sec = ", global_header[4]
  return cfg

def write_header(cfg, outfile):
  nchan = cfg["nchan"]
  bw = cfg["bandwidth"]
  nsubband = cfg["nsubband"]

  header = [["HEADER_START"]]
  hlen = struct.pack('i', len(sys.argv[1]))
  header.append(["rawdatafile", hlen, sys.argv[1]])

  h = cfg["src_name"]
  hlen = struct.pack('i', len(h))
  header.append(["source_name", hlen, h])
  h = struct.pack('i', 0)
  header.append(["machine_id", h])
  h = struct.pack('i', 0)
  header.append(["telescope_id", h])
  h = struct.pack('d', cfg["src_raj"])
  header.append(['src_raj', h])
  h = struct.pack('d', cfg["src_dec"])
  header.append(['src_dej', h])
  h = struct.pack('d', 0.0)
  header.append(['az_start', h])
  header.append(['za_start', h])
  data_type = 1 # 0: raw data, 1 : filterbank
  h=struct.pack('i', data_type)
  header.append(['data_type', h])
  fch1 = cfg["maxfreq"] - (nsubband-eif-1) * bw
  print fch1, bif, eif
  h = struct.pack('d', fch1)
  header.append(['fch1', h])
  foff = -bw / nchan
  h = struct.pack('d', foff)
  header.append(['foff', h])
  h = struct.pack('i', nchan * (eif-bif+1))
  header.append(['nchans', h])
  h = struct.pack('i', 1)
  header.append(['nbeams', h])
  header.append(['ibeam', h])
  h = struct.pack('i', 32)
  header.append(['nbits', h])
  h = struct.pack('d', cfg["mjd"])
  header.append(['tstart', h])
  h = struct.pack('d', cfg["inttime"] / cfg["nsubint"])
  header.append(['tsamp', h])
  h = struct.pack('i', 1)
  header.append(['nifs', h])
  header.append(["HEADER_END"])

  for h in header:
    hlen = struct.pack('i', len(h[0]))
    outfile.write(hlen)
    outfile.write(h[0])
    print h
    for i in h[1:]:
      outfile.write(i)

def get_source(vexfile, start_time):
  scan = get_scan(vexfile, start_time)
  source = vexfile['SCHED'][scan]['source']
  name = vexfile["SOURCE"][source]["source_name"]
  sra = vexfile["SOURCE"][source]["ra"]
  tra = re.split('h|m|s', sra)
  ra =  (int(tra[0])*100 + int(tra[1]))*100. + float(tra[2])
  sdec = vexfile["SOURCE"][source]["dec"]
  tdec = re.split('d|\'|\"', sdec)
  dec =  (int(tdec[0])*100 + int(tdec[1]))*100 + float(tdec[2])
  return name, ra, dec

def get_freq(vexfile, start_time, setup_station):
  scan = get_scan(vexfile, start_time)
  mode = vexfile['SCHED'][scan]['mode']
  if setup_station == '':
    freq = vexfile['MODE'][mode]['FREQ'][0]
  else:
    freq =''
    for f in vexfile['MODE'][mode].getall('FREQ'):
      if setup_station in f[1:]:
        freq = f[0]
    if freq == '':
        raise KeyError(setup_station)
  channels = set()
  for channel in vexfile['FREQ'][freq].getall('chan_def'):
    f0 = float(channel[1].split()[0])
    sb = 0 if (channel[2].strip().upper() == 'L') else 1
    bw = float(channel[3].split()[0])
    print f0 + sb*bw, bw
    channels.add(f0 + sb*bw)
  nsubband = len(channels)
  maxfreq = max(channels) 
  print 'bw = ', bw
  return nsubband, maxfreq, bw


def get_scan(vexfile, start_time):
  for scan in vexfile['SCHED'].iteritems():
    t = scan[1]['start']
    t = [int(x) if x !='' else 0 for x in re.split('y|d|h|m|s', t)]
    scan_start = get_time(t[0], t[1], t[2]*3600+t[3]*60+t[4])
    scan_len = datetime.timedelta(seconds=int(scan[1]['station'][2].rstrip("sec")))
    scan_end = scan_start + scan_len
    if start_time < scan_end:
      return scan[0]
  print "Could not find scan for t = ", start_time
  sys.exit(1)
    

def mjd(year, day_of_year, sec_of_day):
  y = int(year) + 4799
  jdn = 365*y + (y/4) - (y/100) + (y/400) - 31738 - 2400000.5
  
  jdn = int(jdn) + day_of_year - 1
  return jdn + sec_of_day / 86400.

def print_global_header(infile):
  infile.seek(0)
  gheader_buf = infile.read(global_header_size)
  global_header = struct.unpack('i32s2h5i4c',gheader_buf[:64])
  hour = global_header[4] / (60*60)
  minute = (global_header[4]%(60*60))/60
  second = global_header[4]%60
  n = global_header[1].index('\0')
  print "Experiment %s, SFXC version = %s, date = %dy%dd%dh%dm%ds, nchan = %d, int_time = %d"%(global_header[1][:n], global_header[8], global_header[2], global_header[3], hour, minute, second, global_header[5], global_header[6])

def read_autocorrelations(infile, data, cfg, polarization):
  nchan = cfg['nchan']
  nsubband = cfg['nsubband']
  baseline_header = infile.read(baseline_header_size) 
  baseline = fromfile(infile, count=(nchan+1), dtype='float32')
  nbaseline = data.shape[0]

  index = 0
  b = 0
  while (b < nbaseline) and (baseline.size== (nchan+1)):
    bheader = struct.unpack('i4B', baseline_header)
    weight = bheader[0]
    station1 = bheader[1]
    station2 = bheader[2]
    if (station1 != 0) or (station2 != 0):
      print "(station1, station2) = ", station1, station2
    byte = bheader[3]
    pol = byte&3
    sideband = (byte>>2)&1
    freq_nr = byte>>3
    channel_nr = 2*freq_nr + sideband
    global OLDBYTE
    if OLDBYTE != bheader[3]:
      OLDBYTE = bheader[3]
      print 'Changed freq : baseline = ',b, ', pol =', int(pol), 'freq_nr = ', freq_nr,', sideband = ', int(sideband), ', ch_nr = ', channel_nr
    #print integration, bheader
    if (station1 == station2) and ((int(pol)/2+1)&polarization != 0):
      if sideband == 0: 
        vreal = baseline[1:(nchan+1)]
      else:
        vreal = baseline[nchan-1::-1]
      if isnan(vreal).any()==False:
        # We write data in order of decreasing frequency
        inv_ch = nsubband - channel_nr - 1
        data[b, (inv_ch*nchan):((inv_ch+1)*nchan)] += vreal
      else:
        print "b=("+`station1`+", "+`station2`+"), freq_nr = "+`freq_nr`+",sb="+`sideband`+",pol="+`pol`
        print "invalid data (not a number)"
        pdb.set_trace()
    elif (station1 != station2):
      print 'station1(',station1,') != station2(',station2
    b += 1
    if b < nbaseline:
      baseline_header = infile.read(baseline_header_size) 
      baseline = fromfile(infile, count=(nchan+1), dtype='float32')
  return b

def parse_args():
  usage = "Usage : %prog [OPTIONS] <vex file> <cor file 1> ... <cor file N> <output_file>"
  parser = optparse.OptionParser(usage=usage)
  parser.add_option("-i", "--ifs", dest='ifs', type='string', 
                  help='Range of sub-bands to correlate, format first:last. '+
                       'For example -i 0:3 will write the first 4 sub-bands.')
  parser.add_option("-s", "--setup-station", dest='setup_station', type='string',
                    help="Define setup station, the frequency setup of this station is used in the conversion. "+
                         "The default is to use the first station in the vexfile",
                    default="")
  parser.add_option("-b", "--bandpass", dest='bandpass', default=False, action="store_true",
                  help='Apply bandpass')
  parser.add_option("-z", "--zerodm", dest='zerodm', default=False, action="store_true",
                  help='Apply zerodm subtraction')
  parser.add_option("-p", "--pol", dest='pol', type='string', default='I',
                  help='Which polarization to use: R, L, or I (=R+L), default=I')
  (opts, args) = parser.parse_args()

  if opts.ifs == None:
    bif = 0
    eif = -1
  else:
    ifs = opts.ifs.partition(':')
    bif = int(ifs[0])
    if ifs[1] != '':
      eif = int(ifs[2])
    else:
      eif = bif    

  if opts.pol.upper() == 'L':
    pol = 2
  elif opts.pol.upper() == 'R':
    pol = 1
  elif opts.pol.upper() == 'I':
    pol = 3

  infiles = []
  nargs = len(args)
  if nargs < 3:
    parser.error('Invalid number of arguments')
  try:
    for i in range(1, nargs - 1):
      infiles.append(open(args[i], 'rb'))
  except:
    print "Could not open file : " + args[i]
    sys.exit()

  vexfile = vex.Vex(args[0])
  outfile = open(args[-1], 'w')
  return vexfile, infiles, outfile, bif, eif, pol, opts.setup_station, opts.zerodm, opts.bandpass

def get_time(year, day, seconds):
  t = datetime.datetime(year, 1, 1)
  t += datetime.timedelta(days = day-1)
  t += datetime.timedelta(seconds = seconds)
  return t

def read_integration(infile, cfg, polarization):
  nchan = cfg['nchan']
  nsubband = cfg['nsubband']
  nsubint = cfg['nsubint']
  npol = cfg['npol']

  tsheader_buf = infile.read(timeslice_header_size)
  data = zeros([nsubint, nsubband * nchan], dtype=float32)
  if len(tsheader_buf) != timeslice_header_size:
    return data, 0
  #get timeslice header
  timeslice_header = struct.unpack('4i', tsheader_buf)
  current_slice = timeslice_header[0]
  channel = 0
  while (len(tsheader_buf)==timeslice_header_size) and (channel < nsubband*npol):
    # Read UVW  
    nuvw = timeslice_header[2]
    infile.read(uvw_header_size * nuvw)
    # Read the bit statistics
    nstatistics = timeslice_header[3]
    infile.read(stat_header_size * nstatistics)
    # Read the baseline data    
    nbaseline = timeslice_header[1]
    if nbaseline != nsubint:
      print 'Error : invalid number of baseline in corfile (should be equal to the number of subints)'
      sys.exit(1)
    subints_written = read_autocorrelations(infile, data, cfg, polarization)
    # Get next time slice header
    channel += 1
    if channel < nsubband*npol:
      tsheader_buf = infile.read(timeslice_header_size)
      if len(tsheader_buf) == timeslice_header_size:
        timeslice_header = struct.unpack('4i', tsheader_buf)
  # Do bandpass correction
  #for station in stations:
  #  d = data[station]
  #  band = d[:integration].sum(axis=0) / integration + 1
  #  # Normalize the data by average power
  #  d /= band 
  return data, subints_written 

def pad_zeros(outfile, npad, nsubint, nchan):
  pad = zeros([nsubint, nchan*nsubband],dtype=float32)
  for i in xrange(npad):
    pad.tofile(outfile)

def get_bandpass(infile, cfg, polarization):
    # First determine the size of one subint
    infile.seek(global_header_size)
    data, nsubint = read_integration(infile, cfg, polarization)
    pos = infile.tell()
    size = pos - global_header_size
    infile.seek(0, 2)
    endpos = infile.tell()

    # Use 10 seconds in the middle of the scan
    n = endpos / size
    inttime = cfg["inttime"]
    if (n <= 3) or (n*inttime <= 3):
        start = 0
        toread = n
    else:
        m = int(ceil(5./inttime))
        p = int(ceil(1./inttime))
        start = min(p, n/2 - m)
        toread = min(n-p, n/2 + m) - start

    # Make the bandpass
    infile.seek(global_header_size + start*size)
    data, nsubint = read_integration(infile, cfg, polarization)
    bandpass = data.sum(axis=0)
    for i in range(1, toread):
        data, nsubint = read_integration(infile, cfg, polarization)
        bandpass += data.sum(axis=0)
    
    # Now normalize the bandpass per IF
    nsubband = cfg["nsubband"]
    nchan = cfg["nchan"]
    for i in range(nsubband):
        maxnorm = max(bandpass[i*nchan:(i+1)*nchan])
        bandpass /= max(1., maxnorm)
    return bandpass

#########
############################### Main program ##########################
#########
vexfile, infiles, outfile, bif, eif, pol, setup_station, zerodm, dobp = parse_args()

# Read global header
global_header_size = struct.unpack('i', infiles[0].read(4))[0]
cfg = get_configuration(vexfile, infiles[0], setup_station)
if eif == -1:
  eif = cfg["nsubband"] - 1
start_time = cfg["start_time"]
# Write filterbank header
write_header(cfg, outfile)
total_written = 0
for infile in infiles:
  print_global_header(infile)
  infile.seek(0)
  gheader_buf = infile.read(global_header_size)
  global_header = struct.unpack('i32s2h5i4c',gheader_buf[:64])
  new_nchan = global_header[5]
  new_inttime = global_header[6] / 1000000
  new_start_time = get_time(global_header[2], global_header[3], global_header[4])
  if zerodm or dobp:
      bandpass = get_bandpass(infile, cfg, pol)
  infile.seek(global_header_size)
  tsheader_buf = infile.read(timeslice_header_size)
  timeslice_header = struct.unpack('4i', tsheader_buf)
  new_nsubint = timeslice_header[1]
  if total_written > 0:
    # Check input and pad gap beteen scans with zeros
    error = False 
    if cfg["nchan"] != new_nchan:
      print 'Error: number of channels not constant between files'
      error = True
    if cfg["nsubint"] != new_nsubint:
      print 'Error: number of subints per integration differs between files'
      error = True
    if cfg["inttime"] != new_inttime:
      print 'Error : interation time differs between files'
      error = True
    if new_start_time <= start_time:
      print 'Error : Inout files not in ascending time order'
      error = True
    else:
      dt = new_start_time - start_time
      diff = dt.days*86400 + dt.seconds
      if (diff % inttime) != 0:
        print 'Error: consequtive input files have to be an integer number of integration times appart'
        error = True
    if error:
      print 'Current file :', infile.name
      sys.exit(1)
    npad = diff / inttime - nwritten + nskip 
    total_written += npad
    print 'padding ', npad, 'integrations'
    print 'diff, inttime,nwritten,nskip = ', diff, inttime,nwritten,nskip
    pad_zeros(outfile, npad, nsubint, nchan)
  start_time = new_start_time
  nwritten = 0
  nchan = cfg["nchan"]
  infile.seek(global_header_size)
  data, nread = read_integration(infile, cfg, pol)
  while nread == cfg["nsubint"]:
    #Write data
    if (nwritten >= nskip) and (nread > 0):
      # NB: we ordered subbands in reverse order
      start = (cfg["nsubband"] - eif - 1) * nchan
      end = (cfg["nsubband"] - bif) * nchan
      if zerodm:
          print 'The range =', start/nchan, ', to', end/nchan
          for i in range(start/cfg["nchan"], end/cfg["nchan"]):
              cdata = data[:, i*nchan:(i+1)*nchan]
              cbandpass = bandpass[i*nchan:(i+1)*nchan]
              for j in range(data.shape[0]):
                  av = cdata[j].sum() / nchan
                  cdata[j] -= av * cbandpass
      if dobp:
          # Don't blow up band edges too much
          bp = [x if x > 1e-2 else 1. for x in bandpass]
          data /= bp
      data[:, start:end].tofile(outfile)
      dlen = data[:, start:end].size * 1. / cfg["nsubint"]
      print 'subint ', total_written, ' : Wrote ', nread, ' sub-integrations, dlen = ', dlen
    else:
      print 'Skipped subint ', nwritten
    nwritten += 1
    total_written += 1
    data, nread = read_integration(infile, cfg, pol)
