# Standard Python modules
import json
import os
import re
import optparse
import socket
import subprocess
import sys
import time
import urlparse

# MySQL
import MySQLdb as db

# JSON-RPC
import jsonrpclib

# JIVE Python modules
from vex_parser import Vex
from evlbi import DataFlow

input_nodes = {'On': 'mk5-2', 'Wb': 'mk5-15'}

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

# Proper time.
os.environ['TZ'] = "UTC"

usage = "usage: %prog [options] vexfile ctrlfile"
parser = optparse.OptionParser(usage=usage)
parser.add_option("-n", "--nodes", dest="number_nodes",
                  default=128, type="int",
                  help="Number of correlator nodes",
                  metavar="N")
parser.add_option("-m", "--machines", dest="machines",
                  default="a,b,c,d", type="string",
                  help="Machines to run correlator nodes on",
                  metavar="LIST")
parser.add_option("-s", "--simulate", dest="simulate",
                  action="store_true", default=False,
                  help="Simulate")

(options, args) = parser.parse_args()

if len(args) < 2:
    parser.error("incorrect number of arguments")
    pass

vex_file = args[0]
ctrl_file = args[1]

# Parse the VEX file.
vex = Vex(vex_file)
exper = vex['GLOBAL']['EXPER']

fp = open(ctrl_file, 'r')
json_input = json.load(fp)
fp.close()

stations = json_input['stations']
stations.sort()

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

if options.simulate:
    control_host = {}
    for station in stations:
        control_host[station] = machines[0]
        del machines[0]
        continue
    input_host = {}
    for station in stations:
        if station in input_nodes:
            input_host[station] = input_nodes[station]
            continue
        input_host[station] = machines[0]
        del machines[0]
        continue
    flow = DataFlow(vex, control_host, input_host)
else:
    flow = DataFlow(vex)
    pass

manager_node = "head"
output_node = "head"
log_node = "head"
input_nodes = flow.input_host

simulators = {}
readers = {}

def is_mark5b(station):
    return (flow.mk5_mode[station][0] == "ext")

def start_simulator(station):
    args = ["ssh", control_host[station], "bin/mark5_simul"]
    if is_mark5b(station):
        args.append('-B')
        pass
    log = open(station + "-simulator.log", 'w')
    p = subprocess.Popen(args, stdout=log, stderr=log)
    return p

def start_simulators(stations):
    for station in stations:
        simulators[station] = start_simulator(station)
        continue
    return

def stop_simulators():
    for station in simulators:
        args = ["ssh", control_host[station], "pkill", "mark5_simul"]
        subprocess.call(args)
        continue
    for station in simulators:
        simulators[station].wait()
        continue
    return

def start_reader(station):
    args = ["ssh", input_nodes[station], "bin/mk5udp", "-f"]
    if is_mark5b(station):
        args.append("mark5b")
    else:
        args.append("%s:%s" % (flow.mk5_mode[station][0],
                               flow.mk5_mode[station][1]))
        pass
    log = open(station + "-reader.log", 'w')
    p = subprocess.Popen(args, stdout=log, stderr=log)
    return p

def start_readers(stations):
    for station in stations:
        readers[station] = start_reader(station)
        continue
    return

def stop_readers():
    for station in readers:
        args = ["ssh", input_nodes[station], "pkill", "mk5udp"]
        subprocess.call(args)
        continue
    for station in simulators:
        readers[station].wait()
        continue
    return


basename = os.path.splitext(os.path.basename(ctrl_file))[0]
machine_file = basename + ".machines"
log_file = basename + ".log"


try:
    if options.simulate:
        start_simulators(stations)
        pass
    start_readers(stations)

    time.sleep(1)

    flow.setup(stations)

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
        raise Hell
        pass

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
    sfxc = "/home/kettenis/opt/sfxc.mpich/bin/sfxc.sh"
    args = ["mpirun.mpich", "-np", str(number_nodes),
            "-machinefile", machine_file, sfxc, ctrl_file, vex_file]
    log = open(log_file, 'w')
    p = subprocess.Popen(args, stdout=log, stderr=log)

    time.sleep(60)

    flow.start(stations)

    p.wait()

finally:
    flow.stop(stations)

    if options.simulate:
        stop_simulators()
        pass
    stop_readers()

    pass

sys.exit(0)
