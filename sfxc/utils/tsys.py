#! /usr/bin/python

import json
import optparse
import os
import struct
import sys
import time
import urlparse
import numpy as np
import scipy.interpolate
from scipy.special import erfinv

# JIVE Python modules
from vex import Vex
import key

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

def timerang2time(timerang):
    tupletime = (int(timerang[0]), int(timerang[1]), int(timerang[2]),
                 int(timerang[3]), 0, 0, -1, -1, -1)
    return time.mktime(tupletime)

def roundup(n, m):
    return int(((n + (m - 1)) // m) * m)

os.environ['TZ'] = 'UTC'
time.tzset()

header = "=I32sII15sxii"
format = "=BBBBII4Q"

tsys = {}
counts = {}
times = {}

gains = {}

usage = "usage: %prog [options] station [vexfile ctrlfile...]"
parser = optparse.OptionParser(usage=usage)
parser.add_option("-f", "--file", dest="tsys_file",
                  default="", type="string",
                  help="Tsys measurements",
                  metavar="FILE")
parser.add_option("-l", "--lisfile", dest="lis_file",
                  default="", type="string",
                  help="job list file",
                  metavar="FILE")
parser.add_option("-r", "--rxgfile", dest="rxg_file",
                  default="", type="string",
                  help="EVN RXG file",
                  metavar="FILE")
parser.add_option("-t", "--tcalfile", dest="tcal_file",
                  default="", type="string",
                  help="VLBA TCAL file",
                  metavar="FILE")
parser.add_option("-i", "--integration-time", dest="delta_secs",
                  default=20, type="int",
                  help="integration time",
                  metavar="SECS")
parser.add_option("-c", "--cutoff", dest="cutoff",
                  default=1e9, type="float",
                  help="cutoff",
                  metavar="K")
parser.add_option("-e", "--extrapolate", dest="extrapolate",
                  action="store_true", default=False,
                  help="extrapolate Tcal measurements")

(options, args) = parser.parse_args()
if len(args) < 1:
    parser.error("incorrect number of arguments")
    pass

antab_station = args[0]

if options.lis_file:
    fp = open(options.lis_file, 'r')
    line = fp.readline()
    line = line.split()
    exper = line[0]
    vex_file = line[1]
    for line in fp:
        if not line.startswith('+'):
            continue
        line = line.split()
        job = line[1].split('/')[0]
        scan = line[3]
        ctrl_file = "%s/%s_%s.ctrl" % (job, exper, scan)
        break
    fp.close()
else:
    vex_file = args[1]
    ctrl_file = args[2]
    pass

stations = []
vex = Vex(vex_file)
for station in vex['STATION']:
    stations.append(station)
    continue
stations.sort()

# Build a list of start and stop time pairs of each scan
scans = []
for scan in vex['SCHED']:
    start = vex2time(vex['SCHED'][scan]['start'])
    for station in vex['SCHED'][scan].getall('station'):
        if station[0] == antab_station:
            stop = start + int(station[2].split()[0])
            data_good = int(station[1].split()[0])
            scans.append((start + data_good, stop))
            continue
        continue
    continue

if not antab_station in stations:
    print "station %s not found in VEX" % antab_station
    sys.exit(1)
    pass

fp = open(ctrl_file, 'r')
json_input = json.load(fp)
fp.close()

start = vex2time(json_input['start'])
stop = vex2time(json_input['stop'])

if 'setup_station' in json_input:
    setup_station = json_input['setup_station']
else:
    setup_station = json_input['stations'][0]
    pass

if options.rxg_file:
    pol_mapping = {'rcp': 0, 'lcp': 1}
    freqs = {}
    rxgs = {}
    dpfu = [0.0, 0.0]

    fp = open(options.rxg_file)
    lineno = -1
    for line in fp:
        if line.startswith('*'):
            continue
        lineno += 1
        if lineno == 0:
            if line.startswith('range'):
                values = line.split()
                lo = float(values[1])
                hi = float(values[2])
                continue
            if line.startswith('fixed'):
                values = line.split()
                lo = float(values[1])
                hi = float(values[1])
                continue
            pass
        elif lineno == 3:
            pols = line.split()
            continue
        elif lineno == 4:
            values = line.split()
            for pol,value in zip(pols,values):
                dpfu[pol_mapping[pol]] = float(value)
                continue
            continue
        elif lineno == 5:
            mount = line.split()[0]
            poly = [float(x) for x in line.split()[2:]]
            continue
        elif line.startswith('lcp') or line.startswith('rcp'):
            values = line.split()
            if len(values) != 3:
                continue
            pol = pol_mapping[values[0]]
            freq = float(values[1])
            if not pol in freqs:
                freqs[pol] = []
                rxgs[pol] = {}
                pass
            if not freq in freqs[pol]:
                freqs[pol].append(freq)
                pass
            rxgs[pol][freq] = float(values[2])
            continue
        continue
    for pol in freqs:
        freqs[pol] = sorted(freqs[pol])
        continue
    values = {}
    for pol in freqs:
        values[pol] = []
        for freq in freqs[pol]:
            values[pol].append(rxgs[pol][freq])
            continue
        continue
    if options.extrapolate:
        for pol in freqs:
            freqs[pol] = [0.0] + freqs[pol] + [1e12]
            values[pol] = [values[pol][0]] + values[pol] + [values[pol][-1]]
            continue
        pass
    tcals = {}
    for pol in freqs:
        freqs[pol] = np.array(freqs[pol])
        values[pol] = np.array(values[pol])
        values[pol] = scipy.interpolate.interp1d(freqs[pol], values[pol])
        tcals[pol] = values[pol]
        continue
    freq = (lo + hi) / 2
    gains[freq] = {}
    gains[freq][mount] = True
    gains[freq]['DPFU'] = dpfu
    gains[freq]['POLY'] = poly
    gains[freq]['FREQ'] = (lo, hi)
    gains[freq]['FT'] = 1.0
    pass
elif options.tcal_file:
    freqs = []
    tcals = {}

    fp = open(options.tcal_file)
    for line in fp:
        if line.startswith('!'):
            continue
        if line.startswith('RECEIVER'):
            if len(freqs) > 0:
                break
            continue
        values = line.split()
        if len(values) == 0:
            continue
        freq = float(values[0])
        freqs.append(freq)
        tcals[freq] = {1: float(values[1]), 0: float(values[2])}
        continue
    freqs = sorted(freqs)
    rcp = []
    lcp = []
    for freq in freqs:
        rcp.append(tcals[freq][0])
        lcp.append(tcals[freq][1])
        continue
    freqs = np.array(freqs)
    rcp = np.array(rcp)
    lcp = np.array(lcp)
    rcp = scipy.interpolate.interp1d(freqs, rcp)
    lcp = scipy.interpolate.interp1d(freqs, lcp)
    tcals = {0: rcp, 1: lcp}

    fp = open('vlba_gains.key', 'r')
    vlba_gains = key.read_keyfile(fp)
    fp.close()

    for gain in vlba_gains:
        gain_dict = {}
        for pair in gain:
            gain_dict[pair[0]] = pair[1]
            continue
        gain = gain_dict
        if not antab_station.upper() in gain:
            continue
        if start < timerang2time(gain['TIMERANG'][0:4]):
            continue
        if start >= timerang2time(gain['TIMERANG'][4:8]):
            continue
        gains[gain['FREQ']] = gain
        continue
    pass

channels = json_input['channels']

mode = None
for scan in vex['SCHED']:
    if start >= vex2time(vex['SCHED'][scan]['start']):
        mode = vex['SCHED'][scan]['mode']
        pass
    continue

def get_bbc(station, mode):
    for bbc in vex['MODE'][mode].getall('BBC'):
        if station in bbc[1:]:
            return bbc[0]
        continue
    return None

def get_freq(station, mode):
    for freq in vex['MODE'][mode].getall('FREQ'):
        if station in freq[1:]:
            return freq[0]
        continue
    return None

def get_if(station, mode):
    for eyef in vex['MODE'][mode].getall('IF'):
        if station in eyef[1:]:
            return eyef[0]
        continue
    return None

def get_channel_pol(station, mode, chan_name):
    freq = get_freq(station, mode)
    for chan_def in vex['FREQ'][freq].getall('chan_def'):
        if chan_name == chan_def[4]:
            return get_bbc_pol(station, mode, chan_def[5])
        continue
    return None

def get_channel_bw(station, mode, chan_name):
    freq = get_freq(station, mode)
    for chan_def in vex['FREQ'][freq].getall('chan_def'):
        if chan_name == chan_def[4]:
            return float(chan_def[3].split[0])
        continue
    return None

def get_bbc_pol(station, mode, bbc_name):
    bbc = get_bbc(station, mode)
    for bbc_assign in vex['BBC'][bbc].getall('BBC_assign'):
        if bbc_name == bbc_assign[0]:
            return get_if_pol(station, mode, bbc_assign[2])
        continue
    return None

def get_if_pol(station, mode, if_name):
    eyef = get_if(station, mode)
    for if_def in vex['IF'][eyef].getall('if_def'):
        if if_name == if_def[0]:
            return if_def[2]
        continue
    return None

def get_sample_rate(station, mode):
    freq = get_freq(station, mode)
    value = vex['FREQ'][freq]['sample_rate'].split()
    return float(value[0]) * 1e6

pol_mapping = {'R': 0, 'L': 1}
sideband_mapping = {'L': 0, 'U': 1}

freq = get_freq(setup_station, mode)
sample_rate = get_sample_rate(antab_station, mode)

sidebands = []
frequencies = []
num_channels = 0
bandwidth = 0
for chan_def in vex['FREQ'][freq].getall('chan_def'):
    sideband = chan_def[2]
    if not sideband in sidebands:
        sidebands.append(sideband)
        pass
    frequency = float(chan_def[1].split()[0])
    bandwidth = float(chan_def[3].split()[0])
    if sideband == 'L':
        frequency -= bandwidth / 2
    else:
        frequency += bandwidth / 2
    if not frequency in frequencies:
        frequencies.append(frequency)
        pass
    num_channels += 1
    continue
sidebands.sort(reverse=True)
frequencies.sort()

polarisations = []
eyef = get_if(setup_station, mode)
for if_def in vex['IF'][eyef].getall('if_def'):
    polarisation = if_def[2]
    if not polarisation in polarisations:
        polarisations.append(polarisation)
        pass
    continue
polarisations.sort(reverse=True)
num_channels /= len(polarisations)

chan_mapping = {}
for polarisation in polarisations:
    for sideband in sidebands:
        for n in xrange(len(frequencies)):
            chan_mapping[(frequencies[n], sideband, polarisation)] = \
                (n / len(sidebands), sideband_mapping[sideband], pol_mapping[polarisation])
            continue
        continue
    continue

if not 0 in tcals:
    polarisations.remove('R')
    pass
if not 1 in tcals:
    polarisations.remove('L')
    pass

index = []
mapping = {}
comment_mapping = {}
for polarisation in polarisations:
    for n in xrange(num_channels):
        index.append("%s%d" % (polarisation, n + 1))
        frequency = frequencies[n]
        sideband = sidebands[n % len(sidebands)]
        mapping[index[-1]] = chan_mapping[(frequency, sideband, polarisation)]
        comment_mapping[index[-1]] = (frequency, sideband, polarisation)
        continue
    continue

delta = 10000
freq = None
for t in gains:
    if abs(float(t) - frequencies[0]) < delta:
        freq = t
        delta = abs(float(t) - frequencies[0])
        pass
    continue
if freq:
    gain = gains[freq]

    print "GAIN %s" % antab_station.upper(),
    if 'EQUAT' in gain:
        mount = "EQUAT"
    elif 'ALTAZ' in gain:
        mount = "ALTAZ"
    elif 'ELEV' in gain:
        mount = "ELEV"
    elif 'GCNRAO' in gain:
        mount = "GCNRAO"
        pass
    print mount,
    print "DPFU=%#.5g,%#.5g" % tuple(gain['DPFU']),
    try:
        print "FREQ=%.0f" % gain['FREQ']
    except:
        print "FREQ=%.0f,%.0f" % tuple(gain['FREQ'])
    try:
        print "POLY=%#.5g" % gain['POLY']
    except:
        print "POLY=%s" % ','.join(["%#.5g" % (f) for f in gain['POLY']])
    print "/"
    pass

tcal = {}
for polarisation in polarisations:
    for n in xrange(num_channels):
        idx = "%s%d" % (polarisation, n + 1)
        pol = pol_mapping[polarisation]
        tcal[idx] = tcals[pol](frequencies[n])
        continue
    continue

print "TSYS %s" % antab_station.upper(),
try:
    print "FT=%.3f" % gain['FT'],
except:
    print "FT=%.3f" % gain['FT'][0],
print "TIMEOFF=0"
print "INDEX=%s" % ','.join(["'%s'" % (s) for s in index])
print "/"

for i in xrange(len(index)):
    idx = index[i]
    print "!Column %d = %s: %.2f MHz, BW=%.2f MHz, %cSB, Tcal=%.2f K" % \
        (i + 1, idx, comment_mapping[idx][0], bandwidth, comment_mapping[idx][1], tcal[idx])
    continue

if options.lis_file:
    fp = open(options.lis_file, 'r')
    line = fp.readline()
    line = line.split()
    exper = line[0]
    tsys_files = []
    for line in fp:
        if not line.startswith('+'):
            continue
        line = line.split()
        job = line[1].split('/')[0]
        scan = line[3]
        tsys_file = "%s/%s_%s.tsys" % (job, exper, scan)
        tsys_files.append(tsys_file)
        continue
    fp.close()
elif options.tsys_file:
    tsys_files = [options.tsys_file]
else:
    tsys_files = []
    for ctrl_file in args[2:]:
        fp = open(ctrl_file, 'r')
        json_input = json.load(fp)
        fp.close()
        tsys_file = urlparse.urlparse(json_input['tsys_file']).path
        tsys_files.append(tsys_file)
        continue
    pass

scan = 0
binned_secs = scans[0][0]
delta_secs = options.delta_secs

for tsys_file in tsys_files:
    fp = open(tsys_file, 'r')
    buf = fp.read(struct.calcsize(header))
    while fp:
        try:
            buf = fp.read(struct.calcsize(format))
            entry = struct.unpack(format, buf)
        except:
            break
        if (entry[6] + entry[7]) < 500:
            continue
        if (entry[8] + entry[9]) < 500:
            continue

        station = entry[0]
        frequency = entry[1]
        sideband = entry[2]
        polarisation = entry[3]
        mjd = entry[4]
        secs = (mjd - 40587) * 86400 + entry[5]
        if stations[station] != antab_station:
            continue
        if secs < scans[scan][0]:
            continue
        while secs >= scans[scan][1]:
            scan += 1
            continue
        if binned_secs < scans[scan][0]:
            binned_secs = scans[scan][0]
            pass
        while secs >= binned_secs + delta_secs:
            binned_secs += delta_secs
            continue
        if not station in counts:
            counts[station] = {}
            pass
        if not binned_secs in counts[station]:
            counts[station][binned_secs] = {}
            pass

        idx = (frequency, sideband, polarisation)
        if not idx in counts[station][binned_secs]:
            counts[station][binned_secs][idx] = [0, 0, 0, 0]
            pass
        counts[station][binned_secs][idx][0] += entry[6]
        counts[station][binned_secs][idx][1] += entry[7]
        counts[station][binned_secs][idx][2] += entry[8]
        counts[station][binned_secs][idx][3] += entry[9]
        continue
    continue

for station in counts:
    for secs in counts[station]:
        if not station in times:
            times[station] = []
            tsys[station] = {}
            pass
        if not secs in times[station]:
            times[station].append(secs)
            tsys[station][secs] = {}
            pass

        for idx in counts[station][secs]:
            entry = counts[station][secs][idx]
            if (entry[0] + entry[1] + entry[2] + entry[3]) < 0.9 * delta_secs * sample_rate:
                continue
            f_on = float(entry[1])/(entry[0] + entry[1])
            f_off = float(entry[3])/(entry[2] + entry[3])
            P_on = 1 / (2 * (erfinv(1 - f_on))**2)
            P_off = 1 / (2 * (erfinv(1 - f_off))**2)
            if P_on < P_off:
                continue
            P_avg = (P_on + P_off) / 2

            tsys[station][secs][idx] = P_avg/(P_on - P_off)
            continue
        continue
    continue

for station in xrange(len(stations)):
    if stations[station] == antab_station:
        break
    continue

times[station].sort()
for secs in times[station]:
    tupletime = time.gmtime(secs + 0.5 * delta_secs)
    if not tsys[station][secs]:
        continue
    print "%d %02d:%02d.%02d" % (tupletime.tm_yday, tupletime.tm_hour, tupletime.tm_min, ((tupletime.tm_sec * 100) / 60)),
    for idx in index:
        try:
            val = (tsys[station][secs][mapping[idx]] * tcal[idx])
        except:
            val = 999.9
            pass
        if val > options.cutoff:
            val = 999.9
            pass
        print "%.1f" % val,
        continue
    print ""
    continue

print "/"
