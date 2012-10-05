#! /usr/bin/python

# Standard Python modules
import os
import re
import sys
import time

from vex_parser import Vex

import eop
import gps

os.environ['TZ'] = 'UTC'
time.tzset()

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

def get_start(vex):
    sched = vex['SCHED']
    for scan in sched:
        return sched[scan]['start']
    return ""

def get_stop(vex):
    sched = vex['SCHED']
    for scan in sched:
        start = vex2time(sched[scan]['start'])
        duration = int(sched[scan]['station'][2].split()[0])
        continue
    return time2vex(start + duration)

def update(src, dest):
    vex = Vex(src.name)
    exper = vex['GLOBAL']['EXPER']
    start = get_start(vex)
    start = vex2time(start)
    tm = time.gmtime(start - 86400)
    ref_exper = re.compile(r'\s*ref \$EXPER')
    ref_eop = re.compile(r'\s*ref \$EOP')
    ref_das = re.compile(r'\s*ref \$DAS')
    ref_clock = re.compile(r'\s*ref \$CLOCK')
    ref_tapelog_obs = re.compile(r'\s*ref \$TAPELOG_OBS')
    def_station = re.compile(r'\s*def ([a-zA-Z]+);')
    enddef = re.compile(r'\*enddef;')
    block = re.compile(r'\$[A-Z]+;')
    block_eop = re.compile(r'\$EOP;')
    block_clock = re.compile(r'\$CLOCK;')
    block_station = re.compile(r'\$STATION;')
    has_eop = False
    has_clock = False
    has_tapelog_obs = False
    for line in src:
        if ref_eop.match(line):
            has_eop = True
            pass
        if ref_clock.match(line):
            has_clock = True
            pass
        if ref_tapelog_obs.match(line):
            has_tapelog_obs = True
            pass
        continue
    src.seek(0)
    suppress_block = False
    station_block = False
    station = None
    for line in src:
        if not has_eop and ref_exper.match(line):
            dest.write(line)
            dest.write("     ref $EOP = EOP%d;\n" % tm.tm_yday)
            continue
        if ref_eop.match(line):
            dest.write("     ref $EOP = EOP%d;\n" % tm.tm_yday)
            continue
        if ref_clock.match(line):
            dest.write("     ref $CLOCK = %s;\n" % station.upper())
            continue
        if ref_das.match(line):
            if not has_clock:
                dest.write("     ref $CLOCK = %s;\n" % station.upper())
                pass
            if not has_tapelog_obs and ref_das.match(line):
                dest.write("     ref $TAPELOG_OBS = %s;\n" % station.upper())
                pass
            dest.write(line)
            continue
        if block.match(line):
            suppress_block = False
            station_block = False
            pass
        if block_station.match(line):
            station_block = True
            pass
        if station_block and def_station.match(line):
            station = def_station.match(line).group(1)
            pass
        if station_block and enddef.match(line):
            station = None
            pass
        if block_eop.match(line) or block_clock.match(line):
            suppress_block = True
            pass
        if not suppress_block:
            dest.write(line)
            continue

        continue

    dest.write("*" + 77 * "-" + "\n")
    gps.create_clock_block(vex, dest)
    dest.write("*" + 77 * "-" + "\n")
    eop.create_eop_block(vex, dest)
    dest.write("*" + 77 * "-" + "\n")
    if not has_tapelog_obs:
        dest.write("$TAPELOG_OBS;\n")
        for station in vex['STATION']:
            dest.write("*\n")
            dest.write("def %s;\n" % station.upper())
            dest.write("     VSN = 1 : %s-eVLBI : %s : %s;\n"  \
                           % (station, get_start(vex), get_stop(vex)))
            dest.write("enddef;\n")
            continue
        pass
    return

if __name__ == "__main__":
    fp = open(sys.argv[1], "r")
    update(fp, sys.stdout)
    sys.exit(0)
