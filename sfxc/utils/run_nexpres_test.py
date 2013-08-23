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

# JIVE Python modules
from vex_parser import Vex

class EvlbiParams:
    def __init__(self, stations):
        self.stations = stations

        self.mtu = {
            'Bd': 9000,
            'Cm': 9000,
            'Da': 9000,
            'De': 9000,
            'Ef': 9000,
            'Jb': 9000,
            'Mc': 9000,
            'Nt': 9000,
            'On': 9000,
            'Sh': 9000,
            'Sv': 9000,
            'Tr': 9000,
            'Ur': 9000,
            'Wb': 9000,
            'Ys': 9000,
            'Zc': 9000
            }
        self.trackmask = {
            'Zx': 0x000000ff
            }
        return

    pass

broken_nodes = []

manager_node = "head"
output_node = "head"
log_node = "head"

# Where we run the input node processes of SFXC.
input_nodes = {
    'Bd': 'sfxc-e0',
    'Cm': 'sfxc-e0',
    'Da': 'sfxc-e1',
    'De': 'sfxc-e1',
    'Ef': 'sfxc-f0',
    'Jb': 'sfxc-f0',
    'Mc': 'sfxc-f1',
    'Nt': 'sfxc-f1',
    'On': 'sfxc-g0',
    'Sh': 'sfxc-g0',
    'Sv': 'sfxc-g1',
    'Tr': 'sfxc-e1',
    'Ur': 'sfxc-h0',
    'Wb': 'sfxc-h0',
    'Ys': 'sfxc-h1',
    'Zc': 'sfxc-h1'
    }

local_cmd_port = {
    'Bd': 2620,
    'Cm': 2621,
    'Da': 2622,
    'De': 2623,
    'Ef': 2624,
    'Jb': 2625,
    'Mc': 2626,
    'Nt': 2627,
    'On': 2628,
    'Sh': 2629,
    'Sv': 2620,
    'Tr': 2621,
    'Ur': 2622,
    'Wb': 2623,
    'Ys': 2624,
    'Zc': 2625
    }

data_port = {
    'Bd': 2630,
    'Cm': 2631,
    'Da': 2632,
    'De': 2633,
    'Ef': 2634,
    'Jb': 2635,
    'Mc': 2636,
    'Nt': 2637,
    'On': 2638,
    'Sh': 2639,
    'Sv': 2630,
    'Tr': 2631,
    'Ur': 2632,
    'Wb': 2633,
    'Ys': 2634,
    'Zc': 2635
    }
data_socket = {}

# Where we run the jive5ab instances that generate fake data.
simul_nodes = {
    'Bd': 'mk5-1',
    'Cm': 'mk5-3',
    'Da': 'mk5-4',
    'De': 'mk5-4',
    'Ef': 'mk5-4',
    'Jb': 'mk5-5',
    'Mc': 'mk5-6',
    'Nt': 'mk5-7',
    'On': 'mk5-8',
    'Sh': 'mk5-9',
    'Sv': 'mk5-10',
    'Tr': 'mk5-11',
    'Ur': 'mk5-12',
    'Wb': 'mk5-4',
    'Ys': 'mk5-5',
    'Zc': 'mk5-15'
}

simul_nodes = {
    'Mc': 'sfxc-g0',
    'Wb': 'sfxc-g1',
    'Ys': 'sfxc-e0',
    'Zc': 'sfxc-e1'
}

# Mapping between host names and IP addresses.  The cluster nodes at
# JIVE are multi-homed, and we want to make sure we send data over the
# 10GbE network.
connect_ip = {
    'sfxc-e0': '192.42.120.82',
    'sfxc-f0': '192.42.120.83',
    'sfxc-g0': '192.42.120.84',
    'sfxc-h0': '192.42.120.85',
    'sfxc-e1': '192.42.120.86',
    'sfxc-f1': '192.42.120.87',
    'sfxc-g1': '192.42.120.88',
    'sfxc-h1': '192.42.120.89'
}

mark4_stations = ['Cm', 'Da', 'De', 'Mc', 'Nt', 'Jb', 'On', 'Tr']
mark5b_stations = ['Bd', 'Ef', 'Sh', 'Sv', 'Ur', 'Wb', 'Ys', 'Zc']

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
                  default=256, type="int",
                  help="Number of correlator nodes",
                  metavar="N")
parser.add_option("-m", "--machines", dest="machines",
                  default="a,b,c,d,e,f,g,h,i,j", type="string",
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
    if machine in ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j']:
        for unit in [0, 1, 2, 3]:
            machines.append("sfxc-" + machine + str(unit))
            continue
        pass
    else:
        machines.append(machine)
        pass
    continue

# Select input nodes.
for station in json_input["data_sources"]:
    url = urlparse.urlparse(json_input["data_sources"][station][0])
    if url.netloc:
        if url.port:
            data_socket[station] = url.port
        else:
            data_socket[station] = 8888
            pass
    elif url.path:
        data_socket[station] = os.path.dirname(url.path)
    else:
        data_socket[station] = '/tmp/mk5read'
        pass
    continue

readers = {}
reader_slots = {}

def start_reader(station):
    args = ["ssh", input_nodes[station], "taskset", str(hex(reader_slots[station])), "bin/jive5ab", "-m3", "-p", str(local_cmd_port[station])]
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

simulators = {}

def start_simulator(station):
    args = ["ssh", simul_nodes[station], "bin/jive5ab", "-m3"]
    print args
    log = open(station + "-simul.log", 'w')
    p = subprocess.Popen(args, stdout=log, stderr=log)
    return p

def start_simulators(stations):
    for station in stations:
        if not station in simul_nodes:
            continue
        simulators[station] = start_simulator(station)
        continue
    return

def stop_simulators():
    for station in simulators:
        args = ["ssh", simul_nodes[station], "pkill", "jive5ab"]
        subprocess.call(args)
        continue
    for station in simulators:
        simulators[station].wait()
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

def send_remote_command(station, command):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5)
        print station, simul_nodes[station]
        s.connect((simul_nodes[station], 2620))
        print command
        s.send(command)
        data = s.recv(1024)
        print repr(data)
        s.close()
    except:
        pass
    return

def setup_flow(stations):
    params = EvlbiParams(stations)
    for station in stations:
        if station in mark5b_stations:
            send_local_command(station, "mode=ext:0xffffffff:1")
            send_local_command(station, "clock_set=32:int:32")
            send_local_command(station, "net_protocol=udp:1342177728:524288;")
        elif station in mark4_stations:
            send_local_command(station, "mode=mark4:64")
            send_local_command(station, "play_rate=data:16.0")
            send_local_command(station, "net_protocol=udp:1342177728:524288;")
        else:
            raise Hell
        send_local_command(station, "net_port=%d;" % data_port[station])
        if station in params.mtu:
            send_local_command(station, "mtu=%d;" % params.mtu[station])
        else:
            send_local_command(station, "mtu=9000")
            pass
        if station in params.trackmask:
            send_local_command(station, "trackmask=0x%x;" % params.trackmask[station])
            pass
        send_local_command(station, "net2sfxc=open:%s:1;" % data_socket[station])
        continue
    return

def finalize_flow(stations):
    for station in stations:
        send_local_command(station, "net2sfxc=close;")
        continue
    return

def start_flow(stations):
    params = EvlbiParams(stations)
    for station in stations:
        if not station in simul_nodes:
            continue
        if station in mark5b_stations:
            send_remote_command(station, "mode=ext:0xffffffff:1")
            send_remote_command(station, "clock_set=32:int:32")
            send_remote_command(station, "net_protocol=udp;")
        elif station in mark4_stations:
            send_remote_command(station, "mode=mark4:64")
            send_remote_command(station, "play_rate=data:16.0")
            send_remote_command(station, "net_protocol=udp;")
        else:
            raise Hell
        send_remote_command(station, "net_port=%d;" % data_port[station])
        if station in params.mtu:
            send_remote_command(station, "mtu=%d;" % params.mtu[station])
            if params.mtu[station] < 5000:
                send_remote_command(station, "ipd=8;")
            else:
                send_remote_command(station, "ipd=65;")
                pass
            pass
        if station in params.trackmask:
            send_remote_command(station, "trackmask=0x%x;" % params.trackmask[station])
            pass
        send_remote_command(station, "fill2net=connect:%s:0;" % connect_ip[input_nodes[station]])
        time.sleep(2)
        send_remote_command(station, "fill2net=on;")
        continue
    return

def stop_flow(stations):
    for station in stations:
        if not station in simul_nodes:
            continue
        send_remote_command(station, "fill2net=disconnect;")
        continue
    return


basename = os.path.splitext(os.path.basename(ctrl_file))[0]
machine_file = basename + ".machines"
rank_file = basename + ".ranks"
log_file = basename + ".log"

try:
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
            #procs[station] = subprocess.Popen(args, stdout=subprocess.PIPE)
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

    # Create a MPI rank file for the job.
    fp = open(rank_file, 'w')
    print >>fp, "rank 0=", manager_node, "slot=0"
    print >>fp, "rank 1=", output_node, "slot=1"
    print >>fp, "rank 2=", log_node, "slot=2,3"
    rank = 3
    slots = {}
    for machine in machines:
        if machine.startswith('sfxc-i') or machine.startswith('sfxc-j'):
            slots[machine] = 0xffff
        else:
            slots[machine] = 0xff
            pass
        continue
    for station in stations:
        if (slots[input_nodes[station]] & 0x07) == 0x07:
            print >>fp, "rank", str(rank), "=", input_nodes[station], "slot=0,1,2"
            reader_slots[station] = 0x04
            slots[input_nodes[station]] &= ~0x07
        else:
            print >>fp, "rank", str(rank), "=", input_nodes[station], "slot=4,5,6"
            reader_slots[station] = 0x40
            slots[input_nodes[station]] &= ~0x70
            pass
        rank += 1
        continue
    for slot in range(16):
        for machine in machines:
            if machine in broken_nodes:
                continue
            if slots[machine] & (1 << slot) == 0:
                continue
            print >>fp, "rank", str(rank), "=", machine, "slot=", str(slot)
            rank += 1
            continue
        continue
    fp.close()

    number_nodes = rank
    if options.number_nodes < number_nodes - 3 - len(stations):
        number_nodes = 3 + len(stations) + options.number_nodes
        pass

    # Start auxilliary processes.
    start_readers(stations)
    if options.simulate:
        start_simulators(stations)
        pass

    time.sleep(5)

    setup_flow(stations)

    # Start the job.
    sfxc = '/home/kettenis/opt/sfxc.openmpi/bin/sfxc.sh'
    args = ['/home/sfxc/bin/mpirun',
            '--mca', 'btl_tcp_if_include', 'bond0,eth0,eth2.4',
            '--mca', 'oob_tcp_if_exclude', 'eth1,eth2,eth3',
            '--machinefile', machine_file, '--rankfile', rank_file,
            '--np', str(number_nodes), sfxc, ctrl_file, vex_file]
    log = open(log_file, 'w')
    p = subprocess.Popen(args, stdout=log, stderr=log)

    time.sleep(20)

    if options.simulate:
        start_flow(stations)
        pass
    p.wait()

finally:
    if options.simulate:
        stop_flow(stations)
        stop_simulators()
        pass
    finalize_flow(stations)
    stop_readers()

    pass

sys.exit(0)
