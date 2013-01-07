#!/usr/bin/python
from numpy import *
from pylab import *
import numpy.polynomial as polynomial
import sys, struct, datetime, pdb
import vex_parser, vex_time
import parameters
from optparse import OptionParser

try:
  import json
except ImportError:
  import simplejson as json

def read_data(corfile, param):
  global_header_size = param.global_header_size
  timeslice_header_size = parameters.timeslice_header_size
  uvw_header_size = parameters.uvw_header_size
  stat_header_size = parameters.stat_header_size
  baseline_header_size = parameters.baseline_header_size
   
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
  global_header = struct.unpack('i32s2h5i4c', gheader_buf[:64])
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
        pol1 = bheader[3]&1
        pol2 = (bheader[3]&2) >> 1
        #print 'pol1 = ' + `pol1` + ', pol2= ' + `pol2`
        if (station1 != station2) and (pol1 == pol2):
          pol = bheader[3]&1
          sb = (bheader[3]&4)>>2
          freq = bheader[3] >> 3
          #print 'pol = ' + `pol` + ', sb = ' + `sb` + ', freq = ' + `freq`
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
        # Estimate SNR from phase noise
        factor = max(1, N/32)
        vis = rfft(lag[idy])*exp(2j*pi*(arange(0, N+1) * idx/(2.*N)))
        vis2 = array([sum(vis[i:i+factor]) for i in range(0,N-factor,factor)])
        phase = unwrap(arctan2(imag(vis2), real(vis2)))
        M = N / factor
        fx = arange(M/10,M+1-M/10)*pi/M # Only use the inner 80% of the band
        offsets[chan,station1, station2-1] = x
        rates[chan,station1, station2-1] = y * 2 * pi / data.shape[3]
        if rl.sum() > 1e-6:
          snr[chan, station1, station2-1] = phase_snr(fx, phase[M/10:M+1-M/10])
        else:
          snr[chan, station1, station2-1] = 0
        if (chan == 3) and (station1==2) and (station2==-3):
          pdb.set_trace()
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
    # Weight functie : (snr/snr_max) * [(1./16)*x**4/((1./16)*x**4+(1-x)**4)]
    #W[:]=1[chan, bline, :, :]
    for j in range(offsets.shape[1]):
      snrmax = snr[chan,j,:].max()
      if snrmax < 1e-6:
        snrmax = 1
      P = [p_good(s, nchan, 20,0.01) for s in snr[chan,j,:]]
      W[j, :] = [(snr[chan,j,i]/snrmax) * (1./16)*P[i]**4/((1./16)*P[i]**4+(1-P[i])**4) for i in arange(len(P))]
      if(j < ref):
        X[j,0:j] = W[j, 0:j]
        X[j,j] = -sum(W[j,:])
        X[j,j+1:ref] = W[j, j:ref-1]
        X[j,ref:] = W[j, ref:]
      elif(j == ref):
        X[j,:] = W[j, :]
      else:
        X[j,0:ref] = W[j, 0:ref]
        X[j,ref:j-1] = W[j, ref+1:j]
        X[j,j-1] = -sum(W[j,:])
        X[j,j:] = W[j, j:]
      if (chan == -3):
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

def apply_model(data, station1, station2, delays, rates, param,):
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
    if (ch == 0) and (station1==0) and (station2==-1):
      pdb.set_trace()
    bldata[ch] = data[ch] * exp(-2j*pi*f*delay)
    bldata[ch] = (bldata[ch].T * exp(-1j*t*rate)).T
  return bldata

def phase_offsets(data, station1, station2, offsets, rates, snr):
  nchan = data.shape[0]
  nfreq = data.shape[2]
  avfreq = 32
  #pdb.set_trace()
  for chan in range(nchan):
    fringe = irfft2(data[chan, :, :])
    max_idx = abs(fringe).argmax()
    idx = max_idx % fringe.shape[1]
    idy = max_idx / fringe.shape[1]
    vis = rfft(fringe[idy,:])
    # We already did coarse correction, average down in frequency
    factor = max(1, nfreq / avfreq)
    vis2 = array([sum(vis[i:i+factor]) for i in range(0, nfreq-factor, factor)])
    phase = unwrap(arctan2(imag(vis2), real(vis2)))
    N = (phase.size/2) * 2
    x = arange(N/10,N+1-N/10)*pi/N # Only use the inner 80% of the band
    w = abs(vis2[N/10:N+1-N/10])
    wmax = w.max()
    if wmax > 1e-7:
      w /= wmax
    else:
      w[:] = 1
    coef_phase = polynomial.polyfit(x, phase[N/10:N+1-N/10], 1, w=w)
    M = nfreq^1
    vis_rate = sum(data[chan, :, :]*exp(-2j*pi*(arange(0, M+1) * coef_phase[1]/(2.*M))),axis=1)
    phase_rate = unwrap(arctan2(imag(vis_rate), real(vis_rate)))
    w = abs(vis_rate)
    rate_wmax = w.max()
    if rate_wmax > 1e-6:
      w /= rate_wmax
    else:
      w[:] = 1
    coef_rate = polynomial.polyfit(arange(phase_rate.size), phase_rate, 1, w=w)
    offsets[chan,station1, station2-1] = -coef_phase[1]
    rates[chan,station1, station2-1] = -coef_rate[1]
    if (chan == 0) and (station1==0) and (station2==-1):
      pdb.set_trace()
    if wmax > 1e-6:
      snr[chan, station1, station2-1] = phase_snr(x, phase[N/10:N+1-N/10])
    else:
      snr[chan, station1, station2-1] = 0
  #print 'chan = ' + `chan` + ' ; offsets = ' + `offsets[chan,nr,:]`

def phase_snr(x, phase):
  cfit = polynomial.chebyshev.chebfit(x, phase, 6)
  err = phase - polynomial.chebyshev.chebval(x, cfit)
  std = err.std()
  if(std >= 1): 
    #weak signal
    snr = max(0., (1-(sqrt(3)/pi)*std) * sqrt(2*pi**3)/3)
  elif(std >=0.4):
    #intermediate signal strength is done through interpolation
    p1=(0.6-std)*(0.8-std)*(1.0-std)*2.777311908595/(0.2*0.4*0.6)
    p2=(0.4-std)*(0.8-std)*(1.0-std)*2.018309748913/(-0.2*0.2*0.4)
    p3=(0.4-std)*(0.6-std)*(1.0-std)*1.55159280933/(0.4*0.2*0.2)
    p4=(0.4-std)*(0.6-std)*(0.8-std)*1.188669302/(-0.2*0.4*0.6)
    snr = p1 + p2 + p3 + p4
  else:
    #strong signal
    snr = 1 / std
  return 2*snr

def get_options():
  parser = OptionParser('%prog [options] <vex file> <reference station>')
  parser.add_option('-c', '--ctrl', dest='controlfile',
                    help="Use json control file")
  parser.add_option('-f', '--file',
                    dest='corfile',
                    help='Correlator output file')
  parser.add_option('-n', '--max-iter', dest='maxiter', type='int', default='5',
                    help='Maximum number of iterations in fringe search')
  parser.add_option('-p', '--precision', dest='precision', type='float', default='1e-2',
                    help = 'Precision up to which the clock offsets is computed in units of samples')
  parser.add_option('-N', '--no-global', dest='global_fit', action='store_false', default=True,
                    help = 'Do not perform a global fringefit but only use reference station')
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
      sys.exit(1)
  else:
    corfile = options.corfile
  try:
    vex = vex_parser.Vex(vex_name)
  except StandardError, err:
    print >> sys.stderr, "Error loading vex file : " + str(err)
    sys.exit(1)
  return (vex, corfile, ref_station, options.maxiter, options.precision, options.global_fit, ctrl)

def write_clocks(vex, param, delays, rates, snr, global_fit, ref_station):
  vex_stations = [s for s in vex['STATION']]
  # First compute channel weights
  W = zeros(snr.shape)
  for c in range(n_channels):
    for b in range(n_stations):
      snrmax = snr[c,b,:].max()
      if snrmax < 1e-6:
        snrmax = 1
      P = [p_good(s, param.nchan, 20,0.01) for s in snr[c,b,:]]
      W[c, b, :] = [(snr[c,b,i]/snrmax) * (1./16)*P[i]**4/((1./16)*P[i]**4+(1-P[i])**4) for i in range(len(P))]
  if global_fit:
    station_snr = sqrt(sum(snr**2,axis=2))
    weights = sum(W, axis=2)
    weights[:,ref_station] = 0
  else:
    N = snr.shape[1]
    station_snr = zeros(snr.shape[0:2])
    station_snr[:,0:ref_station] = snr[:, ref_station, 0:ref_station]
    station_snr[:,ref_station] = sqrt(sum(snr[:,ref_station,:]**2, axis=1))
    station_snr[:,ref_station+1:N] = snr[:, ref_station, ref_station:N]
    weights = zeros(snr.shape[0:2])
    weights[:,0:ref_station] = W[:, ref_station, 0:ref_station]
    weights[:,ref_station+1:N] = W[:, ref_station, ref_station:N]
  tot_weights = sum(weights,axis=0)
  tot_weights[ref_station] = 1
  weights /= (tot_weights + 1e-6)
  weights[:,ref_station] = 1
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
      delay = -delays[c,s] / (param.sample_rate)
      rate = -sb * rates[c,s]/(2*pi*param.integration_time*(base_freq + sb * param.sample_rate/4))
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
(vex, corfile, ref_station, maxiter, precision, global_fit, ctrl) = get_options() 

vex_stations = [s for s in vex['STATION']]
param = parameters.parameters(vex, corfile) 
try:
  ref_station_nr = vex_stations.index(ref_station)
except:
  print >> sys.stderr, 'Error : reference station ' + ref_station + ' is not found in vex file'
  sys.exit(1)
try:
  ref_index = param.stations.index(ref_station_nr)
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
if global_fit:
  delays, rates = ffit(delay_matrix, rate_matrix, snr, param.nchan, ref_index)
else:
  N = n_stations
  delays[:,0:ref_index]   = delay_matrix[:,ref_index,0:ref_index]
  delays[:,ref_index+1:N] = delay_matrix[:,ref_index,ref_index:N]
  rates[:,0:ref_index]   = rate_matrix[:,ref_index,0:ref_index]
  rates[:,ref_index+1:N] = rate_matrix[:,ref_index,ref_index:N]
it = 1
search_done = True if it >= maxiter else False
while not search_done:
  bline = 0
  snr = zeros([n_channels, n_stations, n_stations-1])
  for station1 in range(n_stations):
    for station2 in range(station1+1, n_stations):
      bldata = apply_model(data[:,bline,:], station1, station2, delays, rates, param)
      phase_offsets(bldata, station1, station2, delay_matrix, rate_matrix, snr)
      bline += 1
  #fill in lower triangle
  for s1 in range(n_stations):
    for s2 in range(0,s1):
      delay_matrix[:,s1,s2] = -delay_matrix[:,s2,s1-1]
      rate_matrix[:,s1,s2] = -rate_matrix[:,s2,s1-1]
      snr[:,s1,s2] = snr[:,s2,s1-1]
  if global_fit:
    ddelays, drates = ffit(delay_matrix, rate_matrix, snr, param.nchan, ref_index)
  else:
    N = n_stations
    ddelays = zeros(delays.shape)
    drates = zeros(rates.shape)
    ddelays[:,0:ref_index]   = delay_matrix[:,ref_index,0:ref_index]
    ddelays[:,ref_index+1:N] = delay_matrix[:,ref_index,ref_index:N]
    drates[:,0:ref_index]   = rate_matrix[:,ref_index,0:ref_index]
    drates[:,ref_index+1:N] = rate_matrix[:,ref_index,ref_index:N]
  delays += ddelays
  rates += drates
  it += 1
  station_snr = sqrt(sum(snr**2,axis=2))
  # 
  filtered_delta = ddelays*((station_snr>10).astype('int'))
  err = abs(filtered_delta).max()
  search_done = True if (err <= precision) or (it >= maxiter) else False
############ Print output in JSON format ################
write_clocks(vex, param, delays, rates, snr, global_fit, ref_index)
