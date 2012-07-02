import os
import time
import sys
import urllib

from vex_parser import Vex

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

def urlopen(url, delta):
    filename = os.path.basename(url)
    try:
        st = os.stat(filename)
    except:
        st = None
        pass
    if (not st or st.st_mtime < (time.time() - delta)):
        urllib.urlretrieve(url, filename)
        pass
    return open(filename)

def tai_utc(t1):
    offset = None
    fp = urlopen("http://maia.usno.navy.mil/ser7/tai-utc.dat", 60 * 86400)
    for line in fp:
        jd = float(line[17:26])
        t2 = (jd - 2440587.5) * 86400
        if t1 >= t2:
            offset = int(line[36:40])
            pass
        continue
    assert offset
    return offset

def eop(t):
    mjd = 40587 + t // 86400
    if abs(time.time() - t) < 90 * 86400:
        fp = urlopen("http://maia.usno.navy.mil/ser7/finals.daily", 6 * 60 * 60)
    else:
        fp = urlopen("http://maia.usno.navy.mil/ser7/finals.data", 6 * 60 * 60)
        pass
    for line in fp:
        if mjd == float(line[7:15]):
            eop = {}
            eop['x_wobble'] = "%2.6f asec" % float(line[18:27])
            eop['y_wobble'] = "%2.6f asec" % float(line[37:46])
            eop['ut1-utc'] = "%2.7f sec" % float(line[58:68])
            eop['delta_psi'] = "%2.6f asec" % (float(line[97:106]) * 1e-3)
            eop['delta_eps'] = "%2.6f asec" % (float(line[116:125]) * 1e-3)
            return eop
        continue
    return

def create_eop_block(vex, fp):
    exper = vex['GLOBAL']['EXPER']
    start = get_start(vex)
    stop = get_stop(vex)
    start = vex2time(start)
    stop = vex2time(stop)
    tm = time.gmtime(start)
    tm = (tm.tm_year, tm.tm_mon, tm.tm_mday, 0, 0, 0, -1, -1, -1)
    eop_points = []
    t = time.mktime(tm) - 86400
    while t < (stop + 86400):
        eop_points.append(eop(t))
        t += 86400
        continue
    assert(len(eop_points) >= 3)

    t = time.mktime(tm) - 86400

    fp.write("$EOP;\n")
    fp.write("def EOP%d;\n" % time.gmtime(t).tm_yday)
    fp.write("   TAI-UTC = %d sec;\n" % tai_utc(t))
    fp.write("   eop_ref_epoch = %s;\n" % time2vex(t))
    fp.write("   eop_interval = 24 hr;\n")
    fp.write("   num_eop_points = %d;\n" % len(eop_points))
    fp.write("   num_nut_points = %d;\n" % len(eop_points))
    for name in ['delta_psi', 'delta_eps', 'x_wobble', 'y_wobble', 'ut1-utc']:
        fp.write("   %s = %s" % (name, eop_points[0][name]))
        for eop_point in eop_points[1:]:
            fp.write(" : %s" % eop_point[name])
            continue
        fp.write(";\n")
        continue
    fp.write("enddef;\n")
    fp.write("\n")
    return


if __name__ == "__main__":
    assert tai_utc(time.mktime((1973, 12, 15, 0, 0, 0, -1, -1, -1))) == 12
    assert tai_utc(time.mktime((2012, 1, 6, 0, 0, 0, -1, -1, -1))) == 34

    vex = Vex(sys.argv[1])
    fp = sys.stdout
    create_eop_block(vex, fp)
    pass
