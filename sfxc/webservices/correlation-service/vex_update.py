#! /usr/bin/python

# Standard Python modules
import os
import re
import sys
import time

import vex

import eop
import gps

os.environ['TZ'] = 'UTC'
time.tzset()

bitstreams_dbbc = [
    (1, 16),
    (2, 24),
    (3, 0),
    (4, 8),
    (5, 18),
    (6, 26),
    (7, 2),
    (8, 10),
    (9, 20),
    (10, 28),
    (11, 4),
    (12, 12),
    (13, 22),
    (14, 30),
    (15, 6),
    (16, 14)
    ]

bitstreams_vlba4 = [
    (1, 16),
    (2, 18),
    (3, 0),
    (4, 2),
    (5, 20),
    (6, 22),
    (7, 4),
    (8, 6),
    (9, 24),
    (10, 26),
    (11, 8),
    (12, 10),
    (13, 28),
    (14, 30),
    (15, 12),
    (16, 14)
    ]

bitstreams_wb = [
    (1, 18),
    (2, 16),
    (3, 2),
    (4, 0),
    (5, 22),
    (6, 20),
    (7, 6),
    (8, 4),
    (9, 26),
    (10, 24),
    (11, 10),
    (12, 8),
    (13, 30),
    (14, 28),
    (15, 14),
    (16, 12)
    ]

bitstreams = {
    'Ef': bitstreams_dbbc,
    'Hh': bitstreams_dbbc,
    'Nt': bitstreams_dbbc,
    'On': bitstreams_dbbc,
    'Sh': bitstreams_vlba4,
    'Tr': bitstreams_dbbc,
    'Wb': bitstreams_wb,
    'Ys': bitstreams_vlba4
    }

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
    v = vex.parse(src.read())
    exper = v['GLOBAL']['EXPER']
    start = get_start(v)
    start = vex2time(start)
    tm = time.gmtime(start - 86400)
    ref_exper = re.compile(r'\s*ref \$EXPER')
    ref_eop = re.compile(r'\s*ref \$EOP')
    ref_das = re.compile(r'\s*ref \$DAS')
    ref_clock = re.compile(r'\s*ref \$CLOCK')
    ref_tapelog_obs = re.compile(r'\s*ref \$TAPELOG_OBS')
    def_station = re.compile(r'\s*def ([a-zA-Z]+);')
    enddef = re.compile(r'\s*enddef;')
    block = re.compile(r'\$[A-Z_]+;')
    block_mode = re.compile(r'\$MODE;')
    block_eop = re.compile(r'\$EOP;')
    block_clock = re.compile(r'\$CLOCK;')
    block_station = re.compile(r'\$STATION;')
    block_site = re.compile(r'\$SITE;')
    site_position_epoch = re.compile(r'\s*site_position_epoch\s*=\s*(\d+);')
    has_eop = False
    has_clock = False
    has_tapelog_obs = False
    has_bitstreams = False
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
    mode_block = False
    station_block = False
    site_block = False
    comment_line = False
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
            mode_block = False
            station_block = False
            site_block = False
            pass
        if block_mode.match(line):
            mode_block = True
            pass
        if mode_block and enddef.match(line):
            if not has_bitstreams:
                for station in v['STATION']:
                    if station in bitstreams:
                        dest.write("     ref $BITSTREAMS = %s:%s;\n" % (station.upper(), station))
                        pass
                    continue
                pass
            mode_block = False
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
        if block_site.match(line):
            site_block = True
            pass
        if site_block and site_position_epoch.match(line):
            epoch = int(site_position_epoch.match(line).group(1))
            secs = (epoch - 40587) * 86400
            tupletime = time.gmtime(secs)
            epoch = time.strftime("%Yy%jd", tupletime)
            dest.write("     site_position_epoch = %s;\n" % epoch)
            continue
        if block_eop.match(line) or block_clock.match(line):
            suppress_block = True
            pass
        if not suppress_block:
            dest.write(line)
            comment_line = line.startswith("*-")
            continue

        continue

    if not has_bitstreams:
        dest.write("*" + 77 * "-" + "\n")
        dest.write("$BITSTREAMS;\n")
        for station in bitstreams:
            dest.write("*\n")
            dest.write("def %s;\n" % station.upper())
            mapping = bitstreams[station]
            for stream in mapping:
                dest.write("     stream_def = &CH%02d : sign : %2d : %2d;\n" % (stream[0], stream[1], stream[1]))
                dest.write("     stream_def = &CH%02d :  mag : %2d : %2d;\n" % (stream[0], stream[1] + 1, stream[1] + 1))
                continue
            dest.write("enddef;\n")
            continue
        pass
    if not has_tapelog_obs:
        dest.write("*" + 77 * "-" + "\n")
        dest.write("$TAPELOG_OBS;\n")
        for station in v['STATION']:
            dest.write("*\n")
            dest.write("def %s;\n" % station.upper())
            dest.write("     VSN = 1 : %s-eVLBI : %s : %s;\n"  \
                           % (station, get_start(v), get_stop(v)))
            dest.write("enddef;\n")
            continue
        pass
    if not comment_line:
        dest.write("*" + 77 * "-" + "\n")
        pass
    gps.create_clock_block(v, dest)
    dest.write("*" + 77 * "-" + "\n")
    eop.create_eop_block(v, dest)
    return

if __name__ == "__main__":
    fp = open(sys.argv[1], "r")
    update(fp, sys.stdout)
    sys.exit(0)
