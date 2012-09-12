#! /usr/bin/python

# Standard Python modules.
import os, re, stat, sys, time
import optparse

# The json module is new in Python 2.6; fall back on simplejson if it
# isn't available.
try:
    import json
except:
    import simplejson as json
    pass

# JIVE Python modules.
from vex_parser import Vex

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

usage = "usage: %prog [options] vexfile"
parser = optparse.OptionParser(usage=usage)
parser.add_option("-i", "--integration-time", dest="integr_time",
                  default=1, type="float", help="Integration time",
                  metavar="SECONDS")
parser.add_option("-c", "--channels", dest="number_channels",
                  default=1024, type="int",
                  help="Number of spectral channels for each subband",
                  metavar="N")
parser.add_option("-d", "--data-dir", dest="data_dir",
                  default="/home/kettenis/mnt", type="string",
                  help="Data directory", metavar="DIR")
parser.add_option("-s", "--stations", dest="stations", type="string",
                  help="Stations to correlate", metavar="LIST")
parser.add_option("-n", "--max-scans", dest="max_scans",
                  default=sys.maxint, type="int",
                  help="Maximem number of scans per job", metavar="N")
parser.add_option("-t", "--template", dest="template", type="string",
                  help="Control file template", metavar="FILE")
parser.add_option("-f", "--fuse", dest="fuse", default=False,
                  action="store_true", help="FUSE based correlation")
parser.add_option("-e", "--evlbi", dest="evlbi", default=False,
                  action="store_true", help="e-VLBI correlation")

(options, args) = parser.parse_args()
if options.stations:
    options.stations = options.stations.split(',')
    pass

if len(args) != 1:
    parser.error("incorrect number of arguments")
    pass

vex_file = args[0]
basename = os.path.splitext(os.path.basename(vex_file))[0]

# Parse the VEX file.
vex = Vex(vex_file)
exper = vex['GLOBAL']['EXPER']

# Proper time.
os.environ['TZ'] = "UTC"

# Generate a list of all media used for this experiment.
media = {}
for station in vex['STATION']:
    try:
        tapelog_obs = vex['STATION'][station]['TAPELOG_OBS']
    except:
        tapelog_obs = None
        pass
    media[station] = []
    if tapelog_obs:
        for vsn in vex['TAPELOG_OBS'][tapelog_obs].getall('VSN'):
            media[station].append({'vsn': vsn[1], 'start': vex2time(vsn[2]),
                                   'stop': vex2time(vsn[3])})
            continue
        pass
    continue

# Create control file template.
json_template = {}
if options.template:
    fp = open(options.template, 'r')
    json_template = json.load(fp)
    fp.close()
    pass

mode = None
stop_time = -1
num_scans = 0
vsn = {}
mk5_scan = {}

# Start with a clean slate.
json_output = json_template.copy()
json_output["stations"] = []
json_output["channels"] = []

for scan in vex['SCHED']:
    if num_scans >= options.max_scans:
        new_job = True
        num_scans = 0
    else:
        new_job = False
        pass

    # Start a new job whenever the mode changes.
    # Figure out the subbands to correlate based on the new mode.
    if mode != vex['SCHED'][scan]['mode']:
        mode = vex['SCHED'][scan]['mode']
        freq = vex['MODE'][mode]['FREQ'][0]
        channels = []
        for channel in vex['FREQ'][freq].getall('chan_def'):
            channels.append(channel[4])
            pass
        new_job = True
        pass

    # Create a list of stations participating in this scan.
    stations = []
    for transfer in vex['SCHED'][scan].getall('station'):
        station = transfer[0]
        if not options.stations or station in options.stations:
            stations.append(station)
            pass
        continue
    stations.sort()

    # A gap signals the start of a new Mark5 scan.  A new Mark5 scan
    # means we have to start a new job.
    if options.fuse and stop_time != vex2time(vex['SCHED'][scan]['start']):
        for station in stations:
            mk5_scan[station] = scan
            continue
        new_job = True
        pass

    # Start a new job whenever a station joins or leaves the
    # observation.
    if json_output["stations"] != stations:
        for station in stations:
            if not station in json_output["stations"]:
                mk5_scan[station] = scan
                continue
        new_job = True
        pass

    # Loop over all the "station" parameters in the scan, figuring out
    # the real length of the scan.
    start_time = sys.maxint
    stop_time = 0
    for transfer in vex['SCHED'][scan].getall('station'):
        station = transfer[0]
        start_time = min(start_time, int(transfer[1].split()[0]))
        stop_time = max(stop_time, int(transfer[2].split()[0]))
        continue

    # Figure out the real start and stop time.
    start_time += vex2time(vex['SCHED'][scan]['start'])
    stop_time += vex2time(vex['SCHED'][scan]['start'])

    # Start a new job whenever there is a media change.
    for station in stations:
        for medium in media[station]:
            if start_time >= medium['start'] and start_time < medium['stop']:
                if not station in vsn or medium['vsn'] != vsn[station]:
                    vsn[station] = medium['vsn']
                    mk5_scan[station] = scan
                    new_job = True
                    break
                pass
            continue
        continue

    # Loop over all the "station" parameters in the scan, figuring out
    # the start offset of the scan on the media.
    offsets = {}
    for transfer in vex['SCHED'][scan].getall('station'):
        station = transfer[0]
        if transfer[3].split()[1].upper() != 'GB':
            raise AssertionError, "Unknown unit " + transfer[3].split()[1]
        offset = int(round(float(transfer[3].split()[0]) * 1e9))
        offsets[station] = offset
        continue

    # Loop over all stations, creating a list of input data files.
    data_sources = {}
    for station in stations:
        if options.fuse:
            data_source = options.data_dir + "/" + exper.lower() + '_' \
                + station.lower() + '_' + mk5_scan[station].lower()
            data_sources[station] = ["file://" + data_source]
        elif options.evlbi:
            data_source = "/tmp/mk5-" + station + "/" + station + "-eVLBI:0"
            data_sources[station] = ["mk5://" + data_source]
        else:
            data_source = vsn[station] + ":" + str(offsets[station])
            data_sources[station] = ["mk5://" + data_source]
        continue

    if new_job:
        # Clean the slate again.
        json_output = json_template.copy()
        json_output["data_sources"] = data_sources
        json_output["start"] = time2vex(start_time)
        json_output["stop"] = time2vex(stop_time)
        json_output["stations"] = stations
        json_output["channels"] = channels

        # Semi-boring stuff.
        json_output["number_channels"] = options.number_channels
        json_output["integr_time"] = options.integr_time
        json_output["exper_name"] = exper
        output_file = basename + '_' + scan.lower() + ".cor"
        output_uri = "file://" + os.getcwd() + "/" + output_file
        json_output["output_file"] =  output_uri
        delay_uri = "file://" + os.getcwd() + "/delays"
        json_output["delay_directory"] = delay_uri

        # Boring stuff.
        json_output["cross_polarize"] = True
        json_output["message_level"] = 1

        # Write the output to a new control file.
        ctrl_file = basename + '_' + scan.lower() + ".ctrl"
    else:
        # Just update the stop time.
        json_output["stop"] = time2vex(stop_time)
        pass

    if options.evlbi:
        json_output["start"] = "now"
        pass

    # Write out the correlator control file.
    fp = open(ctrl_file, 'w')
    json.dump(json_output, fp, indent=4)
    fp.close()

    num_scans += 1
    continue
