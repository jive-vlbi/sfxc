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

urlparse.uses_relative.append('mk5')
urlparse.uses_netloc.append('mk5')
urlparse.uses_params.append('mk5')
urlparse.clear_cache()

# MySQL
import MySQLdb as db

# JSON-RPC
import jsonrpclib

# JIVE Python modules
from vex_parser import Vex

manager_node = "head"
output_node = "head"
log_node = "head"

input_nodes = {
    'Ef': 'sfxc-e0',
    'Mh': 'sfxc-e1',
    'Md': 'sfxc-e1'
    }

local_cmd_port = {
    'Ef': 2620,
    'Mh': 2620,
    'Md': 2620
    }
data_port = {
    'Ef': 2630,
    'Mh': 2630,
    'Md': 2630
    }
data_socket = {}

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
                  default=52, type="int",
                  help="Number of correlator nodes",
                  metavar="N")
parser.add_option("-m", "--machines", dest="machines",
                  default="a,e", type="string",
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
    if machine in ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' ]:
        for unit in [0, 1, 2, 3]:
            node = "sfxc-" + machine + str(unit)
            machines.append("sfxc-" + machine + str(unit))
            continue
        pass
    else:
        machines.append(machine)
        pass
    continue

# Select input nodes.
for machine in options.machines.split(','):
    if machine in ['e', 'f', 'g', 'h']:
        input_nodes['Ef'] = 'sfxc-%c0' % machine
        input_nodes['Mh'] = 'sfxc-%c1' % machine
        input_nodes['Md'] = 'sfxc-%c1' % machine
        pass
    continue

for station in json_input["data_sources"]:
    url = urlparse.urlparse(json_input["data_sources"][station][0])
    if url.path:
        data_socket[station] = os.path.dirname(url.path)
    else:
        data_socket[station] = '/tmp/mk5read'
        pass
    print "datasocket[",station,']=',data_socket[station]
    continue

readers = {}

def start_reader(station):
    args = ["ssh", input_nodes[station], "/home/sfxc/jive5ab_runjob/jive5ab", "-m3", "-p", str(local_cmd_port[station])]
    #args = ["ssh", input_nodes[station], "tmp/evlbi5a/jive5ab", "-m3", "-p", str(local_cmd_port[station])]
    print args
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
        args = ["ssh", input_nodes[station], "pkill", "jive5ab"]
        subprocess.call(args)
        continue
    for station in readers:
        readers[station].wait()
        continue
    return

def send_local_command(station, command):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5)
        print station, input_nodes[station]
        s.connect((input_nodes[station], local_cmd_port[station]))
        print command
        s.send(command)
        data = s.recv(1024)
        print repr(data)
        s.close()
    except:
        pass
    return

def setup_flow(stations):
    for station in stations:
        send_local_command(station, "mode=legacyvdif:8:5000")
        send_local_command(station, "net_protocol=udp:1342177728:524288;")
        send_local_command(station, "net_port=%d;" % data_port[station])
        send_local_command(station, "mtu=9000;")
        send_local_command(station, "net2sfxc=open:%s;" % data_socket[station])
        continue
    return

def finalize_flow(stations):
    for station in stations:
        send_local_command(station, "net2sfxc=close;")
        continue
    return

basename = os.path.splitext(os.path.basename(ctrl_file))[0]
machine_file = basename + ".machines"
rank_file = basename + ".ranks"
log_file = basename + ".log"

try:
    start_readers(stations)

    time.sleep(5)

    finalize_flow(stations)
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
        raise Hell
        pass

    # Create a MPI machine file for the job.
    fp = open(machine_file, 'w')
    print >>fp, manager_node, " slots = 4"
    for machine in machines:
        print >>fp, machine, " slots = 8"
    fp.close()

    # Create a MPI rank file for the job.
    fp = open(rank_file, 'w')
    print >>fp, "rank 0=", manager_node, "slot=0"
    print >>fp, "rank 1=", output_node, "slot=1"
    print >>fp, "rank 2=", log_node, "slot=2,3"
    rank = 3
    for station in stations:
        if data_port[station] % 2 == 0:
            print >>fp, "rank", str(rank), "=", input_nodes[station], "slot=0,1,2,3"
        else:
            print >>fp, "rank", str(rank), "=", input_nodes[station], "slot=4,5,6,7"
        rank += 1
        continue
    for i in range(8):
        for machine in machines:
            if machine in ['sfxc-e0', 'sfxc-f0', 'sfxc-g0', 'sfxc-h0', 'sfxc-e1', 'sfxc-f1', 'sfxc-g1', 'sfxc-h1'] and i < 4:
                continue
            print >>fp, "rank", str(rank), "=", machine, "slot=", str(i)
            rank += 1
            continue
        continue
    fp.close()

    # Start the job.
    number_nodes = 3 + len(stations) + options.number_nodes
    sfxc = '/home/sfxc/bin/sfxc'
    args = ['/home/sfxc/bin/mpirun',
            '--mca', 'btl_tcp_if_include', 'bond0,eth0,eth2.4',
            '--mca', 'oob_tcp_if_exclude', 'eth1,eth2,eth3',
            '--machinefile', machine_file, '--rankfile', rank_file,
            '--np', str(number_nodes), sfxc, ctrl_file, vex_file]
    log = open(log_file, 'w')
    p = subprocess.Popen(args, stdout=log, stderr=log)

    time.sleep(20)

    p.wait()

finally:
    finalize_flow(stations)
    stop_readers()

    pass

sys.exit(0)
