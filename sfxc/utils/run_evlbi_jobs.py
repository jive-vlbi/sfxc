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

sim_input_nodes = {
    'Mc': 'sfxc-c3',
    'On': 'mk5-2',
    'Wb': 'mk5-15' }
sim_remote_ip = {
    'Mc': 'sfxc-c0',
    'On': 'sfxc-c1',
    'Wb': 'sfxc-c2' }
sim_local_ip = {
    'Mc': 'sfxc-c3',
    'On': '10.88.1.202',
    'Wb': '10.88.1.215' }

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

mtu = {}
ipd = {}
remote_ip = {}
def init_evlbi_parameters(stations):
    conn = db.connect(host="ccs", db="ccs", user="jops")
    cursor = conn.cursor()
    for station in stations:
        cursor.execute("SELECT mtu FROM eVLBI_Params" \
                           + " WHERE e_station_name='%s'" % station)
        result = cursor.fetchone()
        if result:
            mtu[station] = result[0]
            pass

        cursor.execute("SELECT ipd FROM eVLBI_Params" \
                           + " WHERE e_station_name='%s'" % station)
        result = cursor.fetchone()
        if result:
            ipd[station] = result[0]
            pass

        cursor.execute("SELECT e_station_control_ip FROM eVLBI_Params" \
                           + " WHERE e_station_name='%s'" % station)
        result = cursor.fetchone()
        if result:
            remote_ip[station] = result[0]
            pass

        continue

    cursor.close()
    conn.close()
    return

# Proper time.
os.environ['TZ'] = "UTC"

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
parser.add_option("-s", "--simulate", dest="simulate",
                  action="store_true", default=False,
                  help="Simulate")

(options, args) = parser.parse_args()

if len(args) < 2:
    parser.error("incorrect number of arguments")
    pass

vex_file = args[0]

# Parse the VEX file.
vex = Vex(vex_file)
exper = vex['GLOBAL']['EXPER']

stations = []
for station in vex['STATION']:
    stations.append(station)
    continue
init_evlbi_parameters(stations)
if options.simulate:
    remote_ip = sim_remote_ip
    pass

fanout = {}
sample_rate = {}
mk5_mode = {}
for station in vex['STATION']:
    das = vex['STATION'][station]['DAS']
    type = vex['DAS'][das]['record_transport_type']
    for scan in vex['SCHED']:
        mode = vex['SCHED'][scan]['mode']

        # Determine the sample rate.
        for freq in vex['MODE'][mode].getall('FREQ'):
            if station in freq[1:]:
                rate = vex['FREQ'][freq[0]]['sample_rate'].split()
                sample_rate[station] = float(rate[0])
                if rate[1] == 'Ks/sec':
                    sample_rate[station] *= 1e3
                elif rate[1] == 'Ms/sec':
                    sample_rate[station] *= 1e6
                    pass
                break
            continue

        # Only look at the $TRACKS section for Mark5A.
        if type == 'Mark5A':
            for tracks in vex['MODE'][mode].getall('TRACKS'):
                if station in tracks[1:]:
                    tracks = vex['TRACKS'][tracks[0]]
                    if not tracks['track_frame_format'] == 'Mark4':
                        break
                    # Count the number of tracks.
                    # Do we need to support fanin as well?
                    sum = 0
                    for fanout_def in tracks.getall('fanout_def'):
                        assert(len(fanout_def) >= 5)
                        sum += (len(fanout_def) - 4)
                        fanout[station] = (len(fanout_def) - 4)
                        continue
                    mk5_mode[station] = ("mark4", "%d" % sum)
                    break
                continue
            pass

        # If we have a $BITSTREAMS section, assume we're dealing
        # with a Mark5B regardless what the VEX file says.
        for bitstreams in vex['MODE'][mode].getall('BITSTREAMS'):
            if station in bitstreams[1:]:
                bitstreams = vex['BITSTREAMS'][bitstreams[0]]
                mask = 0
                for stream_def in bitstreams.getall('stream_def'):
                    mask |= (1 << int(stream_def[2]))
                    continue
                decimation = int(32e6 / sample_rate[station])
                mk5_mode[station] = ("ext", "0x%08x" % mask, "%d" % decimation)
                break
            continue

        if not station in mk5_mode and type == 'Mark5B':
            mask = 0x0000ffff
            decimation = int(32e6 / sample_rate[station])
            mk5_mode[station] = ("ext", "0x%08x" % mask, "%d" % decimation)
            pass
    continue

manager_node = "head"
output_node = "head"
log_node = "head"
input_nodes = {
    'Ar': 'mk5-0',
    'Ef': 'mk5-1',
    'Cm': 'mk5-2',
    'Jb': 'mk5-3',
    'Hh': 'mk5-4',
    'Mc': 'mk5-5',
    'Mh': 'mk5-6',
    'On': 'mk5-7',
    'Sh': 'mk5-8',
    'Tr': 'mk5-9',
    'Wb': 'mk5-10',
    'Ys': 'mk5-11'
}
local_ip = {}
for station in input_nodes:
    id = int(input_nodes[station].split('-')[1])
    local_ip[station] = '192.42.120.%d' % (id * 4 + 2)
    continue
if options.simulate:
    input_nodes = sim_input_nodes
    local_ip = sim_local_ip
    pass

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


simulators = {}
readers = {}

def is_mark5b(station):
    return (mk5_mode[station][0] == "ext")

def start_simulator(station):
    args = ["ssh", remote_ip[station], "bin/mark5_simul"]
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
        args = ["ssh", remote_ip[station], "pkill", "mark5_simul"]
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
        args.append("%s:%s" % (mk5_mode[station][0], mk5_mode[station][1]))
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


def send_simulated_commands(station, commands):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((remote_ip[station], 2620))
    for command in commands:
        print command
        s.send(command)
        data = s.recv(1024)
        print repr(data)
        continue
    s.close()
    return

def send_commands(station, commands):
    if options.simulate:
        send_simulated_commands(station, commands)
        return
    server = jsonrpclib.Server('http://ccsdev:8080')
    results = server.send_commands(station, commands)
    for (command, result) in zip(commands, results):
        print command
        print result
        continue
    return

def start_flow(stations):
    for station in stations:
        send_commands(station, ["in2net=on;"])
        continue
    return

def stop_flow(stations):
    for station in stations:
        send_commands(station, ["in2net=off;"])
        continue
    return

def setup_flow(stations):
    for station in stations:
        send_commands(station, commands[station])
        continue
    return

commands = {}
for station in vex['STATION']:
    # Initialization
    commands[station] = []
    command = "mode=%s" % mk5_mode[station][0]
    for arg in mk5_mode[station][1:]:
        command += ":%s" % arg
        continue
    command += ";"
    commands[station].append(command)
    if not mk5_mode[station][0] == "ext":
        data_rate = sample_rate[station] / (fanout[station] * 1e6)
        command = "play_rate=data:%f;" % data_rate
        commands[station].append(command)
        pass
    command = "net_protocol=udp;"
    commands[station].append(command)
    if station in mtu:
        command = "mtu=%d;" % mtu[station]
        commands[station].append(command)
    if station in ipd:
        command = "ipd=%d;" % ipd[station]
        commands[station].append(command)
    if station in local_ip:
        command = "in2net=connect:%s;" % local_ip[station]
        commands[station].append(command)
        pass

    continue

for ctrl_file in args[1:]:

    basename = os.path.splitext(os.path.basename(ctrl_file))[0]
    machine_file = basename + ".machines"
    log_file = basename + ".log"

    fp = open(ctrl_file, 'r')
    json_input = json.load(fp)
    fp.close()

    stations = json_input['stations']
    stations.sort()

    if options.simulate:
        start_simulators(stations)
        pass
    start_readers(stations)

    time.sleep(1)

    setup_flow(stations)

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

    start_flow(stations)

    p.wait()

    stop_flow(stations)

    if options.simulate:
        stop_simulators()
        pass
    stop_readers()

    continue


sys.exit(0)

if data[0] != '!':
    print "not a valid reply"
    pass

if '=' in data:
    # Command response
    str = data.partition('=')
elif '?' in data:
    # Query response
    str = data.partition('?')
    pass
cmd = str[0]
args = str[2]
str = args.partition(';')
args = str[0].split(':')
for arg in args:
    print arg.strip()
    continue

