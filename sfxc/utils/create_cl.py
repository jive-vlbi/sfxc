#!/usr/bin/env ParselTongue
import AIPS, sys, datetime, struct, pdb
from Wizardry.AIPSData import AIPSUVData
from Wizardry.AIPSData import AIPSTableRow
from numpy import *
from optparse import OptionParser

try:
  import json
except:
  import simplejson as json

def parse_cli_args():
  parser = OptionParser('%prog [options] <config_file>')
  parser.add_option('-v',  '--clver', type='int', default = '0', help = 'The version of the CL table to use')
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
  outfile = open(outname+'.cl', 'w')
  return outfile, uvdata, options.clver

def get_cl(uvdata, outfile, clver):
  cl = uvdata.table('CL', clver)
  nstation = len(uvdata.antennas)
  nif = cl.keywords['NO_IF']
  npol = cl.keywords['NO_POL']
  nchan = uvdata.header.naxis[2]
  start_obs = [int(t) for t in uvdata.header.date_obs.split('-')]
  start_mjd = mjd(start_obs[2], start_obs[1], start_obs[0])

  #A table to convert the AIPS station numbers to SFXC station number
  aips2sfxc = []
  sfxc_antennas = sorted(uvdata.antennas)
  for ant in uvdata.antennas:
    aips2sfxc.append(sfxc_antennas.index(ant))

  # Write global header
  header = struct.pack('5i', start_mjd, nchan, nstation, npol, nif)
  outfile.write(header)

  # Write the IF's to disk
  fq = uvdata.table('FQ', 0)
  f0 = uvdata.header.crval[2] # The reference frequency
  if not(type(fq[0].if_freq) is list):
    frequencies = around(array([f0 + fq[0].if_freq], dtype ='float64'))
  else:
    frequencies = around(array([f + f0 for f in fq[0].if_freq], dtype ='float64'))
  frequencies.tofile(outfile)
  # Store the bandwidths
  bandwidths = array(fq[0].total_bandwidth, dtype='float64')
  bandwidths.tofile(outfile)

  old_time = cl[0].time
  old_interval = cl[0].time_interval
  # stores per IF delay, rate, complex gain, weight, and despersive delay
  row = zeros([nstation,npol,nif,6], dtype='float64') 
  for entry in cl:
    if entry.time != old_time:
      # time in microseconds since midnight
      time_usec  = int64(round(old_time * 86400 * 1000000))
      interval_usec  = int64(round(old_interval * 86400 * 1000000))
      time_str = struct.pack('2q', time_usec, interval_usec)
      # write row to disk
      outfile.write(time_str)
      row.tofile(outfile)
      # create empty new row
      old_time = entry.time
      old_interval = entry.time_interval
      row = zeros([nstation,npol,nif,6], dtype='float64')

    nr = aips2sfxc[entry.antenna_no-1]
    data = [[entry.delay_1, entry.rate_1, entry.real1, entry.imag1, entry.weight_1, entry.disp_1]]
    if npol == 2:
      data.append([entry.delay_2, entry.rate_2, entry.real2, entry.imag2, entry.weight_2, entry.disp_2])
    # Store entry
    for pol, data_row in enumerate(data):
      for if_nr in range(nif):
        for i in range(5):
          if not(type(data_row[i]) is list):
            row[nr, pol, if_nr, i] = data_row[i]
          else:
            row[nr, pol, if_nr, i] = data_row[i][if_nr]
        row[nr, pol, if_nr, 5] = data_row[5]
        print 't =', old_time, ', s = ',nr, ', pol = ', pol, ', if(', if_nr, ') : ', row[nr, pol, if_nr, :]

def mjd(day, month, year):
  a = (14-month)/12;
  y = year + 4800 - a;
  m = month + 12*a - 3;
  jdn = day + ((153*m+2)/5) + 365*y + (y/4) - (y/100) + (y/400) - 32045
  return int(jdn - 2400000.5)

###############################################3
######
###### 

outfile, uvdata, clver = parse_cli_args()
get_cl(uvdata, outfile, clver)
