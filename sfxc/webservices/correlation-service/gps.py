import os
import time
import sys
import urllib

import numpy

from vex_parser import Vex

gps_correction = {
    "HART":      -1.2,
    "JODRELL1":  -0.3,
    "JODRELL2":  +0.3,
    "MEDICINA":  -1.4,
    "NOTO":      -1.4,
    "ONSALA60":  -1.5,
    "ONSALA60_16MHz":  -9.5,
    "ONSALA85":  +1.5,
    "ONSALA85_16MHz":  -6.5,
    "SHANGHAI":  -1.6,
    "TORUN":     -0.4,
    "TORUN_16MHz":     -8.4,
    "WSTRBORK_16Mhz": +10.6,
    "WSTRBORK_8MHz":  +11.1,
    "WSTRBORK_4MHz":  +12.1,
    "WSTRBORK_2MHz":  +14.4,
    "YEBES40M":  -1.4,
}

os.environ['TZ'] = 'UTC'
time.tzset()

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

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

def urlopen(url, delta, filename=None):
    if not filename:
        filename = os.path.basename(url)
        pass
    try:
        st = os.stat(filename)
    except:
        st = None
        pass
    if (not st or st.st_mtime < (time.time() - delta)):
        urllib.urlretrieve(url, filename)
        pass
    return open(filename)

month = ["jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"]

def gps(station, t1):
    x = []; y = []
    range = 4 * 86400
    assert range <= 10 * 86400
    tm = time.gmtime(t1 - range)
    start = (tm.tm_year, tm.tm_mon)
    tm = time.gmtime(t1 - range)
    stop = (tm.tm_year, tm.tm_mon)
    l = [start]
    if not start == stop:
        l.append(stop)
        pass
    for m in l:
        dir = month[m[1] - 1] + str(m[0] % 100)
        url = "ftp://vlbeer.ira.inaf.it/pub/gps/" + dir + \
            "/gps." + station.lower()
        filename = station + "-%d%02d.gps" % m
        try:
            fp = urlopen(url, 12 * 60 * 60, filename)
        except:
           continue

        for line in fp:
            if line.startswith('#'):
                continue
            if not line[0].isdigit():
                continue
            line = line.split()
            mjd = float(line[0])
            t2 = (mjd - 40587) * 86400
            if abs(t1 - t2) <= range:
                x.append(t2)
                y.append(float(line[1]))
                pass
            continue
        continue

    if len(x) > 1:
        return numpy.polyfit(x, y, 1)
    if len(x) == 1:
        return numpy.poly1d([y[0]])
    return numpy.poly1d([0])

def create_clock_block(vex, fp):
    exper = vex['GLOBAL']['EXPER']
    start = get_start(vex)
    stop = get_stop(vex)
    start = vex2time(start)
    stop = vex2time(stop)
    mid = start + (stop - start) / 2

    suffix = None
    for scan in vex['SCHED']:
        mode = vex['SCHED'][scan]['mode']
        freq = vex['MODE'][mode]['FREQ'][0]
        bandwidth = vex['FREQ'][freq]['chan_def'][3].split()
        mult = 1
        if bandwidth[1] == "MHz":
            mult = 1e6
        elif bandwidth[1] == "KHz":
            mult = 1e3
            pass
        bandwidth = float(bandwidth[0]) * mult
        if bandwidth < 3e6:
            suffix = "_2MHz"
        elif bandwidth < 6e6:
            suffix = "_4MHz"
        elif bandwidth < 12e6:
            suffix = "_8MHz"
        else:
            suffix = "_16MHz"
            pass
        break

    fp.write("$CLOCK;\n")
    for station in vex['STATION']:
        site = vex['STATION'][station]['SITE']
        site_name = vex['SITE'][site]['site_name']
        p = gps(station, mid)
        offset = numpy.polyval(p, mid)
        if site_name + suffix in gps_correction:
            offset += gps_correction[site_name + suffix]
            pass
        elif site_name in gps_correction:
            offset += gps_correction[site_name]
            pass
        fp.write("def %s;\n" % station.upper())
        fp.write("   clock_early = %s : %7.3f usec : %s : %10.2e usec/sec;\n" \
                 % (time2vex(start), offset, time2vex(mid), numpy.polyder(p)[0]))
        fp.write("enddef;\n")
        continue
    fp.write("\n");
    return

if __name__ == "__main__":
    vex = Vex(sys.argv[1])
    fp = sys.stdout
    create_clock_block(vex, fp)
    pass
