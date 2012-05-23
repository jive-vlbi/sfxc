#! /usr/bin/python

# Standard Python modules.
import os, re, sys, time

# The json module is new in Python 2.6; fall back on simplejson if it
# isn't available.
try:
    import json
except:
    import simplejson as json
    pass

# JIVE Python modules.
from vex_parser import Vex

# Proper time.
os.environ['TZ'] = "UTC"

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

def get_mode(vex):
    sched = vex['SCHED']
    for scan in sched:
        break
    return sched[scan]['mode']

def get_scan_stop(scan):
    time = vex2time(scan['start'])
    time += max(float(station[2].split()[0]) \
                    for station in scan.getall('station'))
    return time2vex(time)

def get_start(vex):
    sched = vex['SCHED']
    for scan in sched:
        return sched[scan]['start']
    return ""

def get_stop(vex):
    sched = vex['SCHED']
    for scan in sched:
        continue
    return get_scan_stop(sched[scan])

def get_scans(vex):
    sched = vex['SCHED']
    return [(sched[scan]['start'], get_scan_stop(sched[scan])) \
                for scan in sched]

def get_subband(vex, chan_def, IF, BBC):
    result = {}
    result["frequency"] = int(float(chan_def[1].split()[0]) * 1e6)
    result["bandwidth"] = int(float(chan_def[3].split()[0]) * 1e6)
    result["sideband"] = chan_def[2]

    # In order to figure out the polarisation we need the IF this
    # subband is connected to.  The connection between IF and subband
    # is provided by the BBC.
    for BBC_assign in vex["BBC"][BBC].getall("BBC_assign"):
        if BBC_assign[0] == chan_def[5]:
            for if_def in vex["IF"][IF].getall("if_def"):
                if if_def[0] == BBC_assign[2]:
                    result["polarisation"] = if_def[2]
                    break
                continue
            pass
        continue
    return result

def get_subbands(vex):
    mode = get_mode(vex)

    # Pick the first $FREQ block for the mode and find the matching
    # $IF and $BBC blocks.
    freq = vex['MODE'][mode]['FREQ'][0]
    station = vex['MODE'][mode]['FREQ'][1]
    for IF in vex['MODE'][mode].getall('IF'):
        if station in IF[1:]:
            break
        continue
    for BBC in vex['MODE'][mode].getall('BBC'):
        if station in BBC[1:]:
            break
        continue

    result = []
    for chan_def in vex['FREQ'][freq].getall('chan_def'):
        result.append(get_subband(vex, chan_def, IF[0], BBC[0]))
        continue
    return result

def get_channels(vex):
    mode = get_mode(vex)
    freq = vex['MODE'][mode]['FREQ'][0]
    channels = []
    for chan_def in vex['FREQ'][freq].getall('chan_def'):
        channels.append(chan_def[4])
        continue
    return channels

def vex2ccf(vex):
    json_output = {}
    json_output["exper_name"] = vex["GLOBAL"]["EXPER"]
    json_output["start"] = get_start(vex)
    json_output["stop"] = get_stop(vex)
    json_output["scans"] = get_scans(vex)
    json_output["reference_station"] = ""
    json_output["cross_polarize"] = False
    json_output["number_channels"] = 32
    json_output["channels"] = get_channels(vex)
    json_output["subbands"] = get_subbands(vex)
    json_output["integr_time"] = 1.0
    json_output["message_level"] = 1
    json_output["delay_directory"]  = ""
    json_output["output_file"] = ""
    json_output["data_sources"] = {}
    json_output["site_position"] = {}
    json_output["stations"] = []
    for station in vex["STATION"]:
        json_output["stations"].append(station)
        json_output["data_sources"][station] = []
        site = vex["STATION"][station]["SITE"]
        json_output["site_position"][station] = \
            [ float(pos.split()[0]) for pos in vex["SITE"][site]["site_position"] ]
        continue
    return json_output

if __name__ == "__main__":
    vex = Vex(sys.argv[1])
    print json.dumps(vex2ccf(vex), indent=4)
