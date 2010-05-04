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
                  default=1, type="int", help="Integration time",
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
    tapelog_obs = vex['STATION'][station]['TAPELOG_OBS']
    media[station] = []
    for vsn in vex['TAPELOG_OBS'][tapelog_obs].getall('VSN'):
        media[station].append({'vsn': vsn[1], 'start': vex2time(vsn[2]),
                               'stop': vex2time(vsn[3])})
        continue
    continue

mode = None
stop_time = -1
num_scans = 0
vsn = {}

# Start with a clean slate.
json_output = {}
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

    # A gap signals the start of a new Mark5 scan.  A new Mark5 scan
    # means we have to start a new job.
    if stop_time != vex2time(vex['SCHED'][scan]['start']):
        mk5_scan = scan
        new_job = True
        pass

    # Loop over all the "station" parameters in the scan, creating a
    # list of input data files.
    stations = []
    start_time = sys.maxint
    stop_time = 0
    data_sources = {}
    for transfer in vex['SCHED'][scan].getall('station'):
        station = transfer[0]
        start_time = min(start_time, int(transfer[1].split()[0]))
        stop_time = max(stop_time, int(transfer[2].split()[0]))
        data_source = options.data_dir + "/" + exper.lower() + '_' \
            + station.lower() + '_' + mk5_scan.lower()
        data_sources[station] = ["file://" + data_source]
        if not options.stations or station in options.stations:
            stations.append(station)
            pass
        continue

    # Figure out the real start and stop time.
    start_time += vex2time(vex['SCHED'][scan]['start'])
    stop_time += vex2time(vex['SCHED'][scan]['start'])

    # Start a new job whenever a station joins or leaves the
    # observation.
    if json_output["stations"] != stations:
        new_job = True
        pass

    # Start a new job whenever there is a media change.
    for station in stations:
        for medium in media[station]:
            if start_time > medium['start'] and start_time < medium['stop']:
                if station in vsn and medium['vsn'] != vsn[station]:
                    vsn[station] = medium['vsn']
                    new_job = True
                    break
                pass
            continue
        continue

    if new_job:
        # Clean the slate again.
        json_output = {}
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
        delay_uri = "file://" + os.getcwd()
        json_output["delay_directory"] = delay_uri

        # Boring stuff.
        json_output["cross_polarize"] = "true"
        json_output["message_level"] = 1

        # Write the output to a new control file.
        ctrl_file = basename + '_' + scan.lower() + ".ctrl"
    else:
        # Just update the stop time.
        json_output["stop"] = time2vex(stop_time)
        pass

    # Write out the correlator control file.
    fp = open(ctrl_file, 'w')
    json.dump(json_output, fp, indent=4)
    fp.close()

    num_scans += 1
    continue
