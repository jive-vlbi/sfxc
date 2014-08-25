#! /usr/bin/python

# Standard Python modules.
import os, re, stat, sys, time
import optparse, subprocess

# The json module is new in Python 2.6; fall back on simplejson if it
# isn't available.
try:
    import json
except:
    import simplejson as json

# JIVE Python modules.
from vex import Vex

def iso2time(str):
    tupletime = time.strptime(str.rstrip(), "%Y-%m-%d %H:%M:%S")
    return time.mktime(tupletime)

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss")
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

usage = "usage: %prog [options] vexfile"
parser = optparse.OptionParser(usage=usage)
parser.add_option("-i", "--integration-time", dest="integr_time",
                  default=1, type="float", help="Integration time",
                  metavar="SECONDS")
parser.add_option("-S", "--sub-integration-time", dest="sub_integr_time",
                  type="float", help="Sub-integration time",
                  metavar="MICROSECONDS")
parser.add_option("-c", "--channels", dest="number_channels",
                  default=1024, type="int",
                  help="Number of spectral channels for each subband",
                  metavar="N")
parser.add_option("-C", "--cross-polarize", dest="cross_polarize", default=False,
                  action="store_true", help="Enable cross polarizations")
parser.add_option("-s", "--stations", dest="stations", type="string",
                  help="Stations to correlate", metavar="LIST")
parser.add_option("-n", "--max-scans", dest="max_scans",
                  default=sys.maxint, type="int",
                  help="Maximem number of scans per job", metavar="N")
parser.add_option("-t", "--template", dest="template", type="string",
                  help="Control file template", metavar="FILE")
parser.add_option("-o", "--output-directory", dest="output_dir", type="string",
                  help="The directory the correlator output files are written to", 
                  metavar="DIRECTORY")
parser.add_option("-d", "--delay-directory", dest="delay_dir", type="string",
                  help="The directory delay files are written to", 
                  metavar="DIRECTORY")
parser.add_option("-f", "--file-parameters", dest="file_parameters", type='string',
                  help="Specify JSON file which contains locations and filename patterns for stations that are\
                  to be correlated from files, rather than mark5s");
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
output_dir = os.getcwd() if options.output_dir == None else options.output_dir

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
    media[station] = []
    if tapelog_obs:
        for vsn in vex['TAPELOG_OBS'][tapelog_obs].getall('VSN'):
            media[station].append({'vsn': vsn[1], 'start': vex2time(vsn[2]),
                                   'stop': vex2time(vsn[3])})

# Create a dictory in which we store if a station is Mark5A, Mark5B, VLBA or VDIF
media_type = {}
for station in vex['STATION']:
  das = vex['STATION'][station].getall('DAS')
  record_transport_type = vex['DAS'][das]['record_transport_type'].lower()
  if record_transport_type == 'mark5a':
    electronics_rack_type = vex['DAS'][das]['electronics_rack_type'].lower()
    if electronics_rack_type == 'vlba':
      type = 'vlba'
    else:
      type = 'mark5a'
  elif record_transport_type == 'mark5b':
    type = 'mark5b'
  else:
    #Unfortunatly we don't have a proper way to tell if its VDIF format
    type = 'vdif'
  media_type[station] = type

# Create control file template.
json_template = {}
if options.template:
    fp = open(options.template, 'r')
    json_template = json.load(fp)
    fp.close()

# Generate list of files for stations that are correlated from file
media_files = {}
if options.file_parameters != None:
  fp = open(options.file_parameters, 'r')
  file_parameters = json.load(fp)
  for station in file_parameters["file_parameters"]:
    for item in file_parameters["file_parameters"][station]["sources"]:
      t = item.partition(':')
      if t[2] == '':
        print 'Error : badly formated location string in ', options.file_parameters
        sys.exit(1)
      year = int(vex['EXPER'][exper]['exper_nominal_start'].partition('y')[0])
      type = media_type[station]
      print ['ssh', t[0], 'get_file_list.py', '-y', `year`, type, t[2]]
      a = subprocess.Popen(['ssh', t[0], 'get_file_list.py', '-y', `year`, type,\
                            t[2]], stdout=subprocess.PIPE)
      lines = a.stdout.readlines()
      files = []
      for line in lines:
        l = line.partition(' ')
        start_time = iso2time(l[2])
        files.append((l[0], start_time))
      media_files[station] = files
  # Add the file locations to ctrl files for run_jobs.py
  json_template["file_parameters"] = file_parameters["file_parameters"]

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

    # Start a new job whenever the mode changes.
    # Figure out the subbands to correlate based on the new mode.
    if mode != vex['SCHED'][scan]['mode']:
        mode = vex['SCHED'][scan]['mode']
        freq = vex['MODE'][mode]['FREQ'][0]
        channels = []
        for channel in vex['FREQ'][freq].getall('chan_def'):
            channels.append(channel[4])
        new_job = True

    # Create a list of stations participating in this scan.
    stations = []
    for transfer in vex['SCHED'][scan].getall('station'):
        station = transfer[0]
        if not options.stations or station in options.stations:
            stations.append(station)
    stations.sort()

    # Start a new job whenever a station joins or leaves the
    # observation.
    if json_output["stations"] != stations:
        for station in stations:
            if not station in json_output["stations"]:
                mk5_scan[station] = scan
        new_job = True

    # Loop over all the "station" parameters in the scan, figuring out
    # the real length of the scan.
    start_time = sys.maxint
    stop_time = 0
    for transfer in vex['SCHED'][scan].getall('station'):
        station = transfer[0]
        start_time = min(start_time, int(transfer[1].split()[0]))
        stop_time = max(stop_time, int(transfer[2].split()[0]))

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

    # Loop over all the "station" parameters in the scan, figuring out
    # the start offset of the scan on the media.
    offsets = {}
    for transfer in vex['SCHED'][scan].getall('station'):
        station = transfer[0]
        if transfer[3].split()[1] == 'ft':
            offsets[station] = 0
            continue
        if transfer[3].split()[1] != 'GB':
            raise AssertionError, "Unknown unit " + transfer[3].split()[1]
        offset = int(round(float(transfer[3].split()[0]) * 1e9))
        offsets[station] = offset

    # Loop over all stations, creating a list of input data files.
    data_sources = {}
    for station in stations:
        if station in media_files:
            data_source = []
            i=0
            oldfile = None
            for file in media_files[station]:
                if (file[1] > start_time) and (file[1]<stop_time):
                    if(oldfile != None):
                        data_source.append(oldfile[0])
                elif file[1] >= stop_time:
                  break
                oldfile = file
            if(oldfile != None):
                data_source.append(oldfile[0])
            print scan, ' : ', data_source        
            data_sources[station] = data_source
        elif options.evlbi:
            data_source = "/tmp/mk5-" + station + "/" + station + "-eVLBI:0"
            data_sources[station] = ["mk5://" + data_source]
        elif station in vsn:
            data_source = vsn[station] + ":" + str(offsets[station])
            data_sources[station] = ["mk5://" + data_source]
        else:
            data_sources[station] = ["file://"]

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
        if options.sub_integr_time != None:
            json_output["sub_integr_time"] = options.sub_integr_time
        json_output["integr_time"] = options.integr_time
        json_output["exper_name"] = exper
        output_file = basename + '_' + scan.lower() + ".cor"
        output_uri = "file://" + output_dir + "/" + output_file
        json_output["output_file"] =  output_uri
        if options.delay_dir == None:
          delay_uri = "file://" + os.getcwd() + "/delays"
        else:
          delay_uri = "file://" + options.delay_dir
        json_output["delay_directory"] = delay_uri

        # Boring stuff.
        json_output["cross_polarize"] = options.cross_polarize
        json_output["message_level"] = 1

        # Write the output to a new control file.
        ctrl_file = basename + '_' + scan.lower() + ".ctrl"
    else:
        # Just update the stop time.
        json_output["stop"] = time2vex(stop_time)

    if options.evlbi:
        json_output["start"] = "now"

    # Write out the correlator control file.
    fp = open(ctrl_file, 'w')
    json.dump(json_output, fp, indent=4)
    fp.close()

    num_scans += 1
