#!/usr/bin/python
from numpy import *
from pylab import *
import numpy.polynomial as polynomial
import sys, struct, datetime, pdb
import vex_parser, vex_time
from parameters import *
from optparse import OptionParser

if sys.version_info > (2, 5):
  import json
else:
  import simplejson as json

global_header_size = 64
timeslice_header_size = 16
uvw_header_size = 32
stat_header_size = 24
baseline_header_size = 8

#def read_data(nr, stations_in_job, channels, n_integrations, nsubint):
def read_data(corfile, param):
  stations_in_job = param.stations
  n_stations = len(stations_in_job)  
  channels = param.channels
  n_channels = len(channels)
  n_integrations = param.n_integrations
  nsubint = param.nsubint 
  try:
    inputfile = open(corfile, 'rb')
  except:
    print >> sys.stderr, "Error : Could not open " + corfile
    sys.exit(1)

  gheader_buf = inputfile.read(global_header_size)
  global_header = struct.unpack('i32s2h5i4c', gheader_buf)
  nchan = global_header[5]
  integration_time = global_header[6]
  n_baseline = n_stations*(n_stations-1)/2
  data = zeros([n_channels, n_baseline, n_integrations, nchan + 1], dtype='c8')
  #print stations_in_job
  
  for i in range(n_integrations):
    # print `i`+'/'+`n_integrations`
    for j in range(nsubint):
      #print `j`+'/'+`nsubint`
      tsheader_buf = inputfile.read(timeslice_header_size)
      timeslice_header = struct.unpack('4i', tsheader_buf)
      # skip over headers
      nbaseline = timeslice_header[1]
      nuvw = timeslice_header[2]
      nstatistics = timeslice_header[3]
      inputfile.seek(uvw_header_size * nuvw + stat_header_size * nstatistics, 1)
      
      baseline_data_size = (nchan + 1) * 8 # data is complex floats
      baseline_buffer = inputfile.read(nbaseline * (baseline_header_size + baseline_data_size)) 
      index = 0
      baselines = []
      for b in range(nbaseline):
        #print `b`+'/'+`nbaseline`
        bheader = struct.unpack('i4b', baseline_buffer[index:index + baseline_header_size])
        index += baseline_header_size
        station1 = stations_in_job.index(bheader[1])
        station2 = stations_in_job.index(bheader[2])
        #print 's1='+`bheader[1]`+', s2='+`bheader[2]`
        if (station1 != station2):
          pol = bheader[3]&1
          sb = (bheader[3]&4)>>2
          freq = bheader[3] >> 3
          chan = channels.index([freq, sb, pol])
          size_str = `2*nchan+2`+'f'
          #pdb.set_trace()
          values = array(struct.unpack(size_str, baseline_buffer[index:index + baseline_data_size]), dtype='f8')
          if station1 < station2:
            bline = station2 - 1 + station1*(n_stations-2)-station1*(station1-1)/2
            #print 'chan='+`chan`+', bline = ' + `bline`+', shape = ' + `data.shape`+', s1=' + `station1`+', s2='+`station2`
            data[chan, bline, i, :] =  values[0:2*nchan+2:2] + complex64(1j) * values[1:2*nchan+2:2]
          else: 
            bline = station1 - 1 + station2*(n_stations-2)-station2*(station2-1)/2
            #print 'chan='+`chan`+', bline = ' + `bline`+', shape = ' + `data.shape`+', s1=' + `station1`+', s2='+`station2`
            data[chan, bline, i, :] =  values[0:2*nchan+2:2] - complex64(1j) * values[1:2*nchan+2:2]
        index += baseline_data_size
  return data

def lag_offsets(data, n_station, offsets, rates, snr):
  #pdb.set_trace()
  for chan in range(data.shape[0]):
    for station1 in range(n_station):
      for station2 in range(station1+1, n_station):
        # First determine peak in lag space
        bline = station2 - 1 + station1*(n_stations-2)-station1*(station1-1)/2
        lag = irfft2(data[chan, bline, :, :])
        rl = abs(lag)
        max_idx = rl.argmax()
        idx = max_idx % rl.shape[1]
        idy = max_idx / rl.shape[1]
        x = idx if idx < rl.shape[1]/2 else idx - rl.shape[1]
        y = idy if idy < rl.shape[0]/2 else idy - rl.shape[0]
        N = rl.shape[1]/2
        # Use initial estimation to determine delay in fourier space
        vis = rfft(lag[idy])*exp(2j*pi*(arange(0, N+1) * idx/(2.*N)))
        phase = unwrap(arctan2(imag(vis), real(vis)))
        fx = arange(N/10,N+1-N/10)*pi/N # Only use the inner 80% of the band
        w = abs(vis[N/10:N+1-N/10])
        w /= w.max()
        coef_phase = polynomial.polyfit(fx, phase[N/10:N+1-N/10], 1, w=w)
        offsets[chan,station1, station2-1] = -coef_phase[1] + x
        # Use delay estimate to determine rate
        vis_rate = sum(data[chan, bline, :, :]*exp(-2j*pi*(arange(0, N+1) * (coef_phase[1]-idx)/(2.*N))),axis=1)
        T = vis_rate.shape[0]
        vis_rate = (vis_rate.T * exp(2j*pi*arange(-T/2,T/2) * idy * 1. / T)).T
        phase_rate = unwrap(arctan2(imag(vis_rate), real(vis_rate)))
        w = abs(vis_rate)
        w /= w.max()
        coef_rate = polynomial.polyfit(arange(phase_rate.size), phase_rate, 1, w=w)
        rates[chan,station1, station2-1] = -coef_rate[1] + y * 2 * pi / T
        #rates[chan,nr,nr+bline] = idy + y - 1 if idy < fringe.shape[0]/2 else (idy + y - 1)-fringe.shape[0]
        if (chan == 0) and (station1==-1):
          pdb.set_trace()
        snr[chan, station1, station2-1] = phase_snr(fx, phase[N/10:N+1-N/10])
      # Fill in values for the lower triangle of the matrices
      for i in range(0,station1):
        offsets[chan,station1,i] = -offsets[chan,i,station1-1]
        rates[chan,station1,i] = -rates[chan,i,station1-1]
        snr[chan,station1,i] = snr[chan,i,station1-1]
      #print 'chan = ' + `chan` + ' ; offsets = ' + `offsets[chan,nr,:]`

def ffit(offsets, rates, snr, nchan, ref):
  X = zeros([offsets.shape[1], offsets.shape[2]])
  W = zeros([offsets.shape[1], offsets.shape[2]])
  sol_delay = zeros([offsets.shape[0], offsets.shape[1]])
  sol_rate = zeros([offsets.shape[0], offsets.shape[1]])
  #print '**********'
  for chan in range(offsets.shape[0]):
    snrmax = snr[chan,:].max()
    # Weight functie : (snr/snr_max) * [(1./16)*x**4/((1./16)*x**4+(1-x)**4)]
    #W[:]=1[chan, bline, :, :]
    for j in range(offsets.shape[1]):
      P = [p_good(s, nchan, 20,0.01) for s in snr[chan,j,:]]
      W[j, :] = [(snr[chan,j,i]/snrmax) * (1./16)*P[i]**4/((1./16)*P[i]**4+(1-P[i])**4) for i in arange(len(P))]
      if(j < ref):
        X[j,0:j] = -W[j, 0:j]
        X[j,j] = sum(W[j,:])
        X[j,j+1:ref] = -W[j, j:ref-1]
        X[j,ref:] = -W[j, ref:]
      elif(j == ref):
        X[j,:] = -W[j, :]
      else:
        X[j,0:ref] = -W[j, 0:ref]
        X[j,ref:j-1] = -W[j, ref+1:j]
        X[j,j-1] = sum(W[j,:])
        X[j,j:] = -W[j, j:]
      if chan == -1:
        print `W[j, :]`
    #X[:] = -1
    # Solve for the delays
    #pdb.set_trace()
    Y = sum(W*offsets[chan, :], axis=1)
    U,S,V = svd(X,full_matrices=False)
    Sinv = array([1/s if (s > 1e-6) else 0 for s in S])
    M = S.size
    #pdb.set_trace()
    y = dot(V.T,dot(diag(Sinv),(dot(U.T,Y)))) #sum((dot(U.T,Y)*Sinv*V.T),axis=1)
    sol_delay[chan,0:ref] = y[0:ref]
    sol_delay[chan,ref+1:] = y[ref:]
    #Solve for the rates
    Y = sum(W*rates[chan, :], axis=1)
    y = dot(V.T,dot(diag(Sinv),(dot(U.T,Y)))) #sum((dot(U.T,Y)*Sinv*V.T),axis=1)
    sol_rate[chan,0:ref] = y[0:ref]
    sol_rate[chan,ref+1:] = y[ref:]
    #pdb.set_trace()
  #print '**********'
  return sol_delay, sol_rate

def p_good(V, n, max_int, dx):
  if(V > 13): # for V>13 the probility of error at the machine precision
    return 1;
  Z = arange(0,max_int,dx)
  return trapz(Z*exp(-(Z**2+V**2)/2)*i0(Z*V)*(1-exp(-(Z**2)/2))**(n-1),dx=dx)

def apply_model(data, station1, station2, delays, rates, param):
  channels = param.channels
  vex_freqs = param.freqs
  n_station = len(param.stations)
  n_baseline = n_station * (n_station-1) / 2
  
  N = data.shape[2]-1
  T = data.shape[1]
  f=arange(0, N+1) /(2.*N)
  t=arange(0,T) - T/2
  bldata = zeros(data.shape, dtype='c8')
  for ch in range(len(channels)):
    base_freq = vex_freqs[channels[ch][0]]
    rate1 = rates[ch,station1]
    rate2 = rates[ch,station2]
    delay1 = delays[ch,station1] 
    delay2 = delays[ch,station2]
    delay = (delay1 - delay2)
    rate = (rate1 - rate2)
    if((station1 == 0) and (ch == -1)):
      pdb.set_trace()
    bldata[ch] = data[ch] * exp(2j*pi*f*delay)
    bldata[ch] = (bldata[ch].T * exp(1j*t*rate)).T
  return bldata

def phase_offsets(data, station1, station2, offsets, rates, snr):
  nchan = data.shape[0]
  #pdb.set_trace()
  for chan in range(nchan):
    fringe = irfft2(data[chan, :, :])
    max_idx = abs(fringe).argmax()
    idx = max_idx % fringe.shape[1]
    idy = max_idx / fringe.shape[1]
    vis = rfft(fringe[idy,:])
    phase = unwrap(arctan2(imag(vis), real(vis)))
    N = phase.size - 1
    x = arange(N/10,N+1-N/10)*pi/N # Only use the inner 80% of the band
    w = abs(vis[N/10:N+1-N/10])
    w /= w.max()
    coef_phase = polynomial.polyfit(x, phase[N/10:N+1-N/10], 1, w=w)
    offsets[chan,station1, station2-1] = -coef_phase[1]
    vis_rate = sum(data[chan, :, :]*exp(-2j*pi*(arange(0, N+1) * coef_phase[1]/(2.*N))),axis=1)
    phase_rate = unwrap(arctan2(imag(vis_rate), real(vis_rate)))
    w = abs(vis_rate)
    w /= w.max()
    coef_rate = polynomial.polyfit(arange(phase_rate.size), phase_rate, 1, w=w)
    rates[chan,station1, station2-1] = -coef_rate[1]
    #rates[chan,nr,nr+bline] = idy + y - 1 if idy < fringe.shape[0]/2 else (idy + y - 1)-fringe.shape[0]
    if (chan == 0) and (station1==0) and (station2==-1):
      pdb.set_trace()
    snr[chan, station1, station2-1] = phase_snr(x, phase[N/10:N+1-N/10])
  #print 'chan = ' + `chan` + ' ; offsets = ' + `offsets[chan,nr,:]`

def phase_snr(x, phase):
  cfit = polynomial.chebyshev.chebfit(x, phase, 6)
  err = phase - polynomial.chebyshev.chebval(x, cfit)
  return 1. / err.std()

def get_options():
  parser = OptionParser('%prog [options] <vex file> <reference station>')
  parser.add_option('-c', '--ctrl', dest='controlfile',
                    help="Use json control file")
  parser.add_option('-f', '--file',
                    dest='corfile',
                    help='Correlator output file')
  parser.add_option('-n', '--max-iter', dest='maxiter', type='int', default='10',
                    help='Maximum number of iterations in fringe search')
  parser.add_option('-p', '--precision', dest='precision', type='float', default='1e-2',
                    help = 'Precision up to which the clock offsets is computed in units of samples')
  (options, args) = parser.parse_args()
  if len(args) != 2:
    parser.error('invalid number of arguments')
  if not (options.controlfile != None) ^ (options.corfile !=None):
    parser.error('either a control file or a correlator output file needs to be specified.')
  vex_name = args[0]
  ref_station = args[1]
  ctrl = None
  if options.controlfile != None:
    try:
      ctrl = json.load(open(options.controlfile, "r"))
      # TODO : Add file extensions for pulsar binning / multiple phase center
      if ctrl["output_file"][:7] != 'file://':
        raise StandardError('Correlator output_file should start with file://')
      corfile = ctrl["output_file"][7:]
    except StandardError, err:
      print >> sys.stderr, "Error loading control file : " + str(err)
      sys.exit(1);
  else:
    corfile = options.corfile
  vex = vex_parser.Vex(vex_name)
  return (vex, corfile, ref_station, options.maxiter, options.precision, ctrl)

def write_clocks(vex, param, delays, rates, snr):
  vex_stations = [s for s in vex['STATION']]
  # First compute channel weights
  W = zeros(snr.shape)
  snrmax = snr.max()
  for c in range(n_channels):
    for b in range(n_stations):
      P = [p_good(s, param.nchan, 20,0.01) for s in snr[c,b,:]]
      W[c, b, :] = [(snr[c,b,i]/snrmax) * (1./16)*P[i]**4/((1./16)*P[i]**4+(1-P[i])**4) for i in arange(len(P))]
  weights = sum(W, axis=2)
  tot_weights = sum(weights,axis=0)
  weights /= tot_weights
  station_snr = sqrt(sum(snr**2,axis=2))
   
  stations = [vex_stations[i] for i in param.stations]
  channels = [param.channel_names[param.vex_channels.index(c)] for c in param.channels]
  print '{'
  print '\"stations\" : [',
  for s in range(n_stations-1):
    print '\"'+stations[s]+'\", ',
  print '\"'+stations[n_stations-1]+'\"],'
  print '\"channels\" : [',
  for c in range(n_channels-1):
    print '\"'+channels[c]+'\", ',
  print '\"'+channels[n_channels-1]+'\"],'
  tmid = vex_time.get_time(param.starttime) + datetime.timedelta(seconds = param.integration_time*param.n_integrations/2)
  tmid_tuple = tmid.utctimetuple()
  tmid_string = '%dy%dd%dh%dm%ds' %(tmid_tuple[0], tmid_tuple[7], tmid_tuple[3], tmid_tuple[4], tmid_tuple[5])
  print '\"epoch\" : \"'+tmid_string+'\",'
  print '\"clocks\" : {'
  for s in range(n_stations):
    print '    \"' + stations[s] +'\" : {'
    for c in range(n_channels):
      base_freq = param.freqs[param.channels[0][0]]
      sb =  -1 if param.channels[c][1] == 0 else 1
      delay = delays[c,s] / (param.sample_rate)
      rate = sb * rates[c,s]/(2*pi*param.integration_time*(base_freq + sb * param.sample_rate/4))
      snr_tot = station_snr[c,s]
      weight = weights[c,s]
      if (c < n_channels - 1):
        print '        \"' + channels[c] + '\" : [' + `delay` + ', ' + `rate` + ', ' + `snr_tot` + ', ' + `weight` + '],'
      else:
        print '        \"' + channels[c] + '\" : [' + `delay` + ', ' + `rate` + ', ' + `snr_tot` + ', ' + `weight` + ']'
    if(s < n_stations-1):
      print '    },'
    else:
      print '    }'
      print '}'
  print '}'
  
######################## MAIN ##############################3
(vex, corfile, ref_station, maxiter, precision, ctrl) = get_options() 

vex_stations = [s for s in vex['STATION']]
param = parameters(vex, corfile) 
try:
  ref_station_nr = vex_stations.index(ref_station)
except:
  print >> sys.stderr, 'Error : reference station ' + ref_station + ' is not found in vex file'
  sys.exit(1)
try:
  ref_station_index = param.stations.index(ref_station_nr)
except:
  print >> sys.stderr, 'Error : reference station ' + ref_station + ' is not found in the correlator output file'
  sys.exit(1)
n_channels = len(param.channels)
n_stations = len(param.stations)
n_baseline = n_stations*(n_stations-1)/2
delay_matrix = zeros([n_channels, n_stations, n_stations-1])
rate_matrix = zeros([n_channels, n_stations, n_stations-1])
snr = zeros([n_channels, n_stations, n_stations-1])
delays = zeros([n_channels, n_stations])
rates = zeros([n_channels, n_stations])
#print [vex_stations[i] for i in param.stations]

# Read the baseline data
data = read_data(corfile, param)

#Get initial estimate
lag_offsets(data, n_stations, delay_matrix, rate_matrix, snr)
delays, rates = ffit(delay_matrix, rate_matrix, snr, param.nchan, ref_station_index)
#print delays[0,:]
#print rates[0,:]
#delays = zeros([n_channels, n_stations])
#rates = zeros([n_channels, n_stations])
it = 1
search_done = True if it == maxiter else False
while not search_done:
  #print '-----------------------------'
  bline = 0
  snr = zeros([n_channels, n_stations, n_stations-1])
  for station1 in range(n_stations):
    for station2 in range(station1+1, n_stations):
      bldata = apply_model(data[:,bline,:], station1, station2, delays, rates, param)
      phase_offsets(bldata, station1, station2, delay_matrix, rate_matrix, snr)
      bline += 1
  #fill in lower triangle
  #pdb.set_trace()
  for s1 in range(n_stations):
    for s2 in range(0,s1):
      delay_matrix[:,s1,s2] = -delay_matrix[:,s2,s1-1]
      rate_matrix[:,s1,s2] = -rate_matrix[:,s2,s1-1]
      snr[:,s1,s2] = snr[:,s2,s1-1]
  #pdb.set_trace()
  tdelays, trates = ffit(delay_matrix, rate_matrix, snr, param.nchan, ref_station_index)
  delays += tdelays
  rates += trates
  #print delays[0,:]
  #print rates[0,:]
  it += 1
  station_snr = sqrt(sum(snr**2,axis=2))
  # 
  filtered_delta = tdelays*((station_snr>10).astype('int'))
  err = abs(filtered_delta).max()
  search_done = True if (err <= precision) or (it >= maxiter) else False
  #print 'iter = ' + `it` + ' / ' + `maxiter` + ' ; err = ' + `err`

############ Print output in JSON format ################
write_clocks(vex, param, delays, rates, snr)
