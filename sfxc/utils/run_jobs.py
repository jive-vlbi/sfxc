#! /usr/bin/python

# Standard Python modules.
import os, re, stat, sys, time
import subprocess
import urlparse
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
parser.add_option("-n", "--nodes", dest="number_nodes",
                  default=128, type="int",
                  help="Number of correlator nodes",
                  metavar="N")
parser.add_option("-m", "--machines", dest="machines",
                  default="a,b,c,d", type="string",
                  help="Machines to run correlator nodes on",
                  metavar="LIST")

(options, args) = parser.parse_args()

if len(args) < 2:
    parser.error("incorrect number of arguments")
    pass

vex_file = args[0]

# Parse the VEX file.
vex = Vex(vex_file)
exper = vex['GLOBAL']['EXPER']

# Proper time.
os.environ['TZ'] = "UTC"

mk5s = ['mk5-' + str(x) for x in range(17)]
manager_node = "head"
output_node = "head"
log_node = "head"

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

# Generate a list of machines to use.
machines = []
for machine in options.machines.split(','):
    if machine in ['a', 'b', 'c', 'd']:
        for unit in [0, 1, 2, 3]:
            machines.append("sfxc-" + machine + str(unit))
            continue
        pass
    else:
        machines.append(machine)
        pass
    continue

for ctrl_file in args[1:]:

    basename = os.path.splitext(os.path.basename(ctrl_file))[0]
    machine_file = basename + ".machines"
    log_file = basename +".log"

    fp = open(ctrl_file, 'r')
    json_input = json.load(fp)
    fp.close()

    start = vex2time(json_input['start'])

    stations = json_input['stations']
    stations.sort()

    # Make sure the delay files are up to date, and generate new ones
    # if they're not.
    procs = {}
    success = True
    for station in stations:
        path = urlparse.urlparse(json_input['delay_directory']).path
        delay_file = path + '/' +  exper + '_' + station + '.del'
        if not os.access(delay_file, os.R_OK) or \
           os.stat(delay_file).st_mtime < os.stat(vex_file).st_mtime:
            args = ['generate_delay_model', vex_file, station, delay_file]
            procs[station] = subprocess.Popen(args, stdout=subprocess.PIPE)
            pass
        continue
    for station in procs:
        output = procs[station].communicate()[0]
        procs[station].wait()
        if procs[station].returncode != 0:
            print "Delay model couldn't be generated for " + station + ":"
            print output
            path = urlparse.urlparse(json_input['delay_directory']).path
            delay_file = path + '/' +  exper + '_' + station + '.del'
            os.remove(delay_file)
            success = False
            pass
        continue
    if not success:
        sys.exit(1)
        pass

    # Figure out the directory where the input data files are located.
    # Should be the same for all stations otherwise things will become
    # way too complicated.
    data_dir = None
    for station in stations:
        data_source = json_input['data_sources'][station][0]
        path = urlparse.urlparse(data_source).path
        if not data_dir:
            data_dir = os.path.dirname(path)
            pass
        assert data_dir == os.path.dirname(path)
        continue

    # Check if the input data files are there.  Do this in a loop that
    # gets repeated until all files have been found.
    missing = True
    while missing:
        # For every Mark5, generate a list of files present.  This is
        # faster than checking each file individually.
        data_dir_list = {}
        for mk5 in mk5s:
            args = ['/usr/bin/ssh', mk5, '/bin/ls ' + data_dir]
            p = subprocess.Popen(args, stdout=subprocess.PIPE)
            output = p.communicate()[0]
            p.wait()
            if p.returncode == 0:
                data_dir_list[mk5] = output.split()
            else:
                data_dir_list[mk5] = []
                pass
            continue

        # For each station, figure out on which Mark5 the input data
        # is located.
        input_nodes = {}
        for station in stations:
            data_source = json_input['data_sources'][station][0]
            path = urlparse.urlparse(data_source).path
            file = os.path.basename(path)
    
            for mk5 in mk5s:
                if file in data_dir_list[mk5]:
                    input_nodes[station] = mk5
                    break
                continue
            continue

        # Check if we found them all.  If not, give the operator a
        # chance to mount the missing media.
        missing = False
        for station in stations:
            if not station in input_nodes:
                if not missing:
                    print "Please mount media with the following VSNs:"
                    missing = True
                    pass
                for medium in media[station]:
                    if start >= medium['start'] and start < medium['stop']:
                        print medium['vsn']
                        pass
                    pass
                pass
            continue

        if missing:
            raw_input("Ready? ")
            pass
        continue

    # Create a MPI machine file for the job.
    fp = open(machine_file, 'w')
    print >>fp, manager_node
    print >>fp, output_node
    print >>fp, log_node
    for station in stations:
        print >>fp, input_nodes[station], "#", station
        continue
    for i in range(8):
        for machine in machines:
            print >>fp, machine
            continue
        continue
    fp.close()

    # Start the job.
    number_nodes = 3 + len(stations) + options.number_nodes
    sfxc = "sfxc"
    cmd = "mpirun.mpich -np " + str(number_nodes) + " " \
        + "-machinefile " + machine_file + " " \
        + sfxc + " " + ctrl_file + " " + vex_file \
        + " 2>&1 | tee " + log_file
    os.system(cmd)

    continue
