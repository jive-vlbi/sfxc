import json
import optparse
import os
import struct
import sys
import time
import urlparse
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

os.environ['TZ'] = 'UTC'
time.tzset()

header = "=I32sII15sxii"
format = "=BBBBII4Q"

tsys = {}
times = {}

# Values for Ef in  F13C4
tcal = {
    'R1': 1.69,
    'R2': 1.69,
    'R3': 1.69,
    'R4': 1.70,
    'R5': 1.74,
    'R6': 1.70,
    'R7': 1.80,
    'R8': 1.78,
    'L1': 1.69,
    'L2': 1.69,
    'L3': 1.68,
    'L4': 1.70,
    'L5': 1.74,
    'L6': 1.70,
    'L7': 1.80,
    'L8': 1.77
    }

gains = {}

usage = "usage: %prog [options] vexfile ctrlfile"
parser = optparse.OptionParser(usage=usage)
parser.add_option("-f", "--file", dest="tsys_file",
                      default="", type="string",
                      help="Tsys measurements",
                      metavar="FILE")

(options, args) = parser.parse_args()
if len(args) < 3:
    parser.error("incorrect number of arguments")
    pass

vex_file = args[0]
ctrl_file = args[1]
antab_station = args[2]

stations = []
vex = Vex(vex_file)
for station in vex['STATION']:
    stations.append(station)
    continue
stations.sort()

if not antab_station in stations:
    print "station %s not found in VEX" % antab_station
    sys.exit(1)
    pass

fp = open(ctrl_file, 'r')
json_input = json.load(fp)
fp.close()

start = vex2time(json_input['start'])
stop = vex2time(json_input['stop'])

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

if 'setup_station' in json_input:
    setup_station = json_input['setup_station']
else:
    setup_station = station[0]
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

pol_mapping = {'R': 0, 'L': 1}
sideband_mapping = {'L': 0, 'U': 1}

freq = get_freq(setup_station, mode)

sidebands = []
frequencies = []
num_channels = 0
for chan_def in vex['FREQ'][freq].getall('chan_def'):
    sideband = chan_def[2]
    if not sideband in sidebands:
        sidebands.append(sideband)
        pass
    frequency = float(chan_def[1].split()[0])
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
                (n, sideband_mapping[sideband], pol_mapping[polarisation])
            continue
        continue
    continue

index = []
mapping = {}
comment_mapping = {}
for polarisation in polarisations:
    for n in xrange(num_channels):
        index.append("%s%d" % (polarisation, n + 1))
        frequency = frequencies[n / len(sidebands)]
        sideband = sidebands[n % len(sidebands)]
        mapping[index[-1]] = chan_mapping[(frequency, sideband, polarisation)]
        comment_mapping[index[-1]] = (frequency, sideband, polarisation)
        continue
    continue

delta = 1000
freq = None
for t in gains:
    if abs(float(t) - frequencies[0]) < delta:
        freq = t
        delta = abs(float(t) - frequencies[0])
        pass
    continue
if freq:
    gain = gains[freq]
    tcal = {}
    for polarisation in polarisations:
        for n in xrange(num_channels):
            idx = "%s%d" % (polarisation, n + 1)
            tcal[idx] = gain['TCAL'][pol_mapping[polarisation]]
            continue
        continue

    print "GAIN %s" % antab_station.upper()
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
    print "DPFU = %.3f, %.3f" % tuple(gain['DPFU']),
    try:
        print "POLY %.4g" % gain['POLY']
    except:
        print gain['POLY']
        print "POLY %s" % ','.join(["%.4g" % (f) for f in gain['POLY']])
    print "/"
    pass

print "TSYS %s" % antab_station.upper()
print "INDEX = %s" % ', '.join(["'%s'" % (s) for s in index])
print "/"

for i in xrange(len(index)):
    idx = index[i]
    print "!Column %d = %s: %.2f MHz, %cSB" % \
        (i + 1, idx, comment_mapping[idx][0], comment_mapping[idx][1])
    continue

first = None
last = None
if options.tsys_file:
    tsys_file = options.tsys_file
else:
    tsys_file = urlparse.urlparse(json_input['tsys_file']).path
    pass
fp = open(tsys_file, 'r')
buf = fp.read(struct.calcsize(header))
while fp:
    try:
        buf = fp.read(struct.calcsize(format))
        entry = struct.unpack(format, buf)
    except:
        break
    if (entry[6] + entry[7]) < 5000:
        continue
    if (entry[8] + entry[9]) < 5000:
        continue
    f_on = float(entry[7])/(entry[6] + entry[7])
    f_off = float(entry[9])/(entry[8] + entry[9])
    P_on = 1 / (2 * (erfinv(1 - f_on))**2)
    P_off = 1 / (2 * (erfinv(1 - f_off))**2)
    if P_on < P_off:
        continue
    P_avg = (P_on + P_off) / 2
    station = entry[0]
    frequency = entry[1]
    sideband = entry[2]
    polarisation = entry[3]
    mjd = entry[4]
    secs = entry[5]
    secs = (mjd - 40587) * 86400 + secs
    if not first:
        first = secs
        pass
    if not last:
        last = secs
        pass
    if secs < first:
        first = secs
        pass
    if secs > last:
        last = secs
        pass
    if stations[station] != antab_station:
        continue
    if not station in times:
        times[station] = []
        tsys[station] = {}
        pass
    if not secs in times[station]:
        times[station].append(secs)
        tsys[station][secs] = {}
        pass
    tsys[station][secs][(frequency, sideband, polarisation)] = P_avg/(P_on - P_off)
    continue

for station in xrange(len(stations)):
    if stations[station] == antab_station:
        break
    continue

times[station].sort()
for secs in times[station]:
    tupletime = time.gmtime(secs)
    print "%d %02d:%02d.%02d" % (tupletime.tm_yday, tupletime.tm_hour, tupletime.tm_min, ((tupletime.tm_sec * 100) / 60)),
    for idx in index:
        try:
            print "%.1f" % (tsys[station][secs][mapping[idx]] * tcal[idx]),
        except:
            print "999.9",
            pass
        continue
    print ""
    continue

print "/"
