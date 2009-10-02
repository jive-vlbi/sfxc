#!/usr/bin/python

# 2007, Developed by Nico Kruithof <Kruithof@jive.nl>

# 2009-05-27 (iii) small:
# Multidict has a tendency to throw away duplicate keys
# so it is especially useless for locating channels;
# I've papered over with grep, but really we should use a 
# _real_ vex parser

import sys, re, os
#from vex_parser import Vex
from Vex import Vex
import simplejson, tempfile
import pprint

def add_time(time, delta):
    start_date = re.match("(\d*)y(\d*)d(\d*)h(\d*)m(\d*)s", time).groups()
    second = int(start_date[4]) + int(delta)
    minute = int(start_date[3]) + (second / 60)
    hour   = int(start_date[2]) + (minute / 60)
    day    = int(start_date[1]) + (hour   / 24)
    year   = int(start_date[0])
    days_per_year = 365
    if (year % 4 == 0) and ((year % 100 != 0) or (year % 400 == 0)):
        days_per_year = 366
        year = year + (day / days_per_year)
    return "%04dy%03dd%02dh%02dm%02ds" % \
        (year, (day%days_per_year), (hour%24), (minute%60), (second%60))

def get_start(vex):
    start = ""
    sched = vex["SCHED"]
    for scan in sched:
        if start == "":
            start = sched[scan]["start"]
    else:
        if start > sched[scan]["start"]:
            start = sched[scan]["start"]
    return start

def get_scan_end(scan):
    return add_time(scan["start"], 
                        max([i[2].split()[0] for i in scan.getall("station")]))

def get_stop(vex):
    sched = vex["SCHED"]
    endtimes = [get_scan_end(sched[scan]) for scan in sched]
    return max(endtimes)

def get_scan_ranges(vex):
    sched = vex["SCHED"]
    return [(sched[scan]['start'], get_scan_end(sched[scan])) for scan in sched]

def get_frequency(vex, IF, BBC, channel):
    if_def = ""
    for bbc in vex["BBC"][BBC].getall("BBC_assign"):
        if bbc[0] == channel[5]:
            if_def = bbc[2]
            
            result = dict()
            result["frequency"]    = float(channel[1].split()[0])*1000000
            result["bandwidth"]    = float(channel[3].split()[0])*1000000
            result["sideband"]     = channel[2]+"SB"
            for if_def_it in vex["IF"][IF].getall("if_def"):
                if if_def_it[0] == if_def:
                    result["polarisation"] = if_def_it[2]
    return result

def get_frequencies(vex):
    # take the first frequency, even if there are multiple frequencies
    for i in vex["MODE"]: mode = i
    IF = vex["MODE"][mode]["IF"][0]
    ref_station = vex["MODE"][mode]["IF"][1]
    BBC = ""
    ## Find the corresponding BBC:
    for bbc in vex["MODE"][mode].getall("BBC"):
        for station in bbc[1:]:
            if station == ref_station:
                BBC = bbc[0]

                result = []
                for freq in vex["FREQ"]:
                    for channel in vex["FREQ"][freq].getall("chan_def"):
                        result.append(get_frequency(vex, IF, BBC, channel))
                break
    return result


def vex2dict(vex):
    json_output = dict()
    json_output["exper_name"] = vex["GLOBAL"]["EXPER"]
    json_output["start"] = get_start(vex)
    json_output["stop"] = get_stop(vex)
    json_output["scans"] = get_scan_ranges(vex)
    json_output["reference_station"] = ""
    json_output["cross_polarize"]    = False
    json_output["number_channels"]   = 1024
    json_output["subbands"] = get_frequencies(vex)
    json_output["integr_time"]       = 1.0
    json_output["message_level"]     = 1
    json_output["delay_directory"]   = ""
    json_output["output_file"]       = ""
    json_output["data_sources"]      = dict()
    json_output["site_position"]     = dict()
    json_output["stations"]          = []
    for station in vex["STATION"]:
        json_output["stations"].append(station)
        json_output["data_sources"][station] = []
        site = vex["STATION"][station]["SITE"]
        json_output["site_position"][station] = [float(pos.split()[0])
                                                 for pos in 
                                                 vex["SITE"][site]["site_position"] ]
    return json_output


import popen2
def vex2ccf(vex_fn):
    vex = Vex(vex_fn)
    pout, pin = popen2.popen2("grep -o '&CH[0-9]*' %s | sort | uniq" % vex_fn)
    channels = [l.strip() for l in pout.readlines()]
    vexdict = vex2dict(vex)
    vexdict['channels'] = channels
    return simplejson.dumps(vexdict)

from vex2ccf_services import *

def convertRequest(vex_content):
    print >>sys.stderr, "Got request"
    vex = vex_content['vex_file']
    tfn = tempfile.mktemp()
    tf = file(tfn, 'w')
    tf.write(vex)
    tf.write('\n')
    tf.close()
    print >>sys.stderr, "************************************"
    s = vex2ccf(tfn)
    os.unlink(tfn)
    response = convertResponse()
    response._ccf_file = unicode(s)
    return response

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "%s takes a vex-file as argument" % sys.argv[0]
        sys.exit(1)
    f = file(sys.argv[1])
    #s = f.read()
    #print vex2ccf(s)
    print vex2ccf(sys.argv[1])
