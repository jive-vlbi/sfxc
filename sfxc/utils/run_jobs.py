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
from vex import Vex

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

usage = "usage: %prog [options] vexfile controlfiles"
parser = optparse.OptionParser(usage=usage)
parser.add_option("-n", "--nodes", dest="number_nodes",
                  default=256,
                  type="int", 
                  help="Number of correlator nodes",
                  metavar="N")
#default_subnet = "10.88.0.0/24,192.168.1.0/24"
default_subnet = "10.88.0.0/24"
parser.add_option("-s", "--subnet", dest="subnet",
                  help='Subnet for MPI communication, default=%s'%default_subnet,
                  default = default_subnet)
parser.add_option("-d", "--no-delays", dest="gen_delays",
                  default = True,
                  action = "store_false",
                  help="Disable generating new delays")
parser.add_option("-m", "--machines", dest="machines",
                  default="a,b,c,d,e,f,g,h,i,j,k", type="string",
                  help="Machines to run correlator nodes on",
                  metavar="LIST")
parser.add_option("-e", "--exclude", dest="exclude_nodes",
                  help="List of node to exclude from correlation",
                  metavar="LIST")
default_head_node = "out.sfxc"
parser.add_option("-H", "--head-node", dest="head_node",
                  default=default_head_node,
                  help="Head node which gets the manager, log, and " +
                        "output node. Default=%s"%default_head_node,
                  metavar="NODE")
parser.add_option("-M", "--on-mark5", help="Correlate with input node on the mark5s", 
                  default= False, action="store_true")
(options, args) = parser.parse_args()

if len(args) < 2:
    parser.error("incorrect number of arguments")

ncore_manager = 8 if options.head_node == 'out.sfxc' else 4

vex_file = args[0]
# Parse the VEX file.
vex = Vex(vex_file)
exper = vex['GLOBAL']['EXPER']

# Proper time.
os.environ['TZ'] = "UTC"

mk5s = ['10.88.0.' + str(50+x) for x in range(17)]
mk5s += ['10.88.0.' + str(50+x) for x in range(20,26)]
mk5s.remove('10.88.0.63') # still away

manager_node = options.head_node
output_node = options.head_node
log_node = options.head_node

# Generate a list of all media used for this experiment.
media = {}
for station in vex['STATION']:
    tapelog_obs = vex['STATION'][station]['TAPELOG_OBS']
    media[station] = []
    for vsn in vex['TAPELOG_OBS'][tapelog_obs].getall('VSN'):
        media[station].append({'vsn': vsn[1], 'start': vex2time(vsn[2]),
                               'stop': vex2time(vsn[3])})

# Generate a list of machines to use.
machines = []
for machine in options.machines.split(','):
    if machine in ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k']:
        for unit in [0, 1, 2, 3]:
            machines.append("sfxc-" + machine + str(unit) + ".sfxc")
    else:
        machines.append(machine)
# Remove nodes which are excluded from job
if options.exclude_nodes != None:
  nodes = ["sfxc-%s.sfxc"%node for node in options.exclude_nodes.split(',')]
  for node in nodes:
    try:
      machines.remove(node)
    except ValueError:
      pass

# Nodes with a 10Gb interface
machines_10g = []
for unit in [0, 1, 2, 3]:
  machines_10g.append("sfxc-k" + str(unit) + ".sfxc")
for machine in ['e', 'f', 'g', 'h']:
  for unit in [0, 1]:
    machines_10g.append("sfxc-" + machine + str(unit) + ".sfxc")

# Sort input nodes in opposite order as the production enviroment
machines_10g = sorted(set(machines_10g).intersection(machines), reverse=True)
print machines_10g

for ctrl_file in args[1:]:
    basename = os.path.splitext(os.path.basename(ctrl_file))[0]
    machine_file = basename + ".machines"
    rank_file = basename + ".ranks"
    log_file = basename +".log"

    fp = open(ctrl_file, 'r')
    json_input = json.load(fp)
    fp.close()
    if 'file_parameters' not in json_input:
      json_input['file_parameters'] = {}
    start = vex2time(json_input['start'])

    stations = json_input['stations']
    stations.sort()

    # Make sure the delay files are up to date, and generate new ones
    # if they're not.
    if options.gen_delays:
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
    use_mark5 = False
    for station in stations:
      if station not in json_input['file_parameters']:
        data_source = json_input['data_sources'][station][0]
        if urlparse.urlparse(data_source).scheme == 'file':
            path = urlparse.urlparse(data_source).path
            if not data_dir:
                data_dir = os.path.dirname(path)
            assert data_dir == os.path.dirname(path)
        elif urlparse.urlparse(data_source).scheme == 'mk5':
          use_mark5 = True

    # Check if the input data files are there.  Do this in a loop that
    # gets repeated until all files have been found.
    missing = True
    while missing:
        if data_dir:
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
            pass
        elif use_mark5:
            # For every Mark5, generate a list of VSNs present.  This is
            # faster than checking each VSN individually.
            vsn_list = {}
            processes = {}
            for mk5 in mk5s:
                args = ['/usr/bin/ssh', mk5, 'bin/vsnread']
                processes[mk5] = subprocess.Popen(args, stdout=subprocess.PIPE)
            for mk5 in mk5s:
                p = processes[mk5]
                output = p.communicate()[0]
                p.wait()
                if p.returncode == 0:
                    vsn_list[mk5] = output.split()
                else:
                    vsn_list[mk5] = []
                    pass
                continue

            # For each station, figure out on which Mark5 the input data
            # is located.
            input_nodes = {}
            for station in stations:
                data_source = json_input['data_sources'][station][0]
                path = urlparse.urlparse(data_source).path
                path = path.lstrip('/')
                if path == "":
                  path = urlparse.urlparse(data_source).netloc
                vsn = path.split(':')[0]
                for mk5 in mk5s:
                    if vsn in vsn_list[mk5]:
                        input_nodes[station] = mk5
                        break

        # Check if we found them all.  If not, give the operator a
        # chance to mount the missing media.
        missing = False
        for station in stations:
          if station not in json_input['file_parameters']:
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

    # Initialize ranks
    ranks = {}
    # Create a MPI machine file for the job.
    fp = open(machine_file, 'w')
    print >>fp, manager_node, "slots="+`ncore_manager` #Assume manager, output, and log node on the same machine
    #print >>fp, output_node
    #print >>fp, log_node
    if options.on_mark5:
      for station in stations:
        if station not in json_input['file_parameters']:
          #ifhn = "ifhn="+input_nodes[station]
          print >>fp, " #", station
          print >>fp, input_nodes[station], " slots=4"
          ranks[input_nodes[station]]  = 4
    for station in json_input['file_parameters']:
      source = json_input['file_parameters'][station]['sources'][0]
      machine = source.partition(':')[0]
      if machine.startswith('sfxc-'):
        if not machine.endswith('.sfxc'):
          machine += '.sfxc'
          if (machine not in machines) and (machine not in ranks):
            m = machine.split('-')
            n = '8' if m[1][0] < 'i' else '16'
            print >>fp, machine, " slots="+n
            ranks[machine] = int(n)
      elif machine.startswith('aribox') or (machine == '10.88.0.24'):
        if machine not in ranks:
          print >>fp, machine, " slots=16"
          ranks[machine] = 16
      elif machine.startswith('flexbuf'):
        if machine not in ranks:
          print >>fp, machine, " slots=12"
          ranks[machine] = 12

    #for i in range(8):
    for machine in machines:
        m = machine.split('-')
        n = '8' if m[1][0] < 'i' else '16'
        print >>fp, machine, " slots="+n
        ranks[machine] = int(n)
    fp.close()

    # Create a MPI rank file for the job.
    fp = open(rank_file, 'w')
    print >>fp, "rank 0=", manager_node, "slot=0"
    print >>fp, "rank 1=", log_node, "slot=1"
    print >>fp, "rank 2=", output_node, "slot=2,3"
    rank=2
    # Create ranks
    idx_10g = 0
    for station in stations:
      rank += 1
      if station in json_input['file_parameters']:
        source = json_input['file_parameters'][station]['sources'][0]
        node = source.partition(':')[0]
        if node.startswith('sfxc-') and not node.endswith('.sfxc'):
            node += '.sfxc'
        nthread = json_input['file_parameters'][station]['nthreads']
        slots = 'slot=' + ','.join([str(i) for i in range(ranks[node]-nthread, ranks[node])])
        ranks[node] -= nthread
        print >>fp, "rank", str(rank), "=", node, slots
      elif options.on_mark5:
        print >>fp, "rank", str(rank), "=", input_nodes[station], "slot=0,1,2"
      else:
        nthread = 2 # FIXME don't hardcode this
        node = machines_10g[idx_10g]
        idx_10g = (idx_10g + 1)%len(machines_10g)
        slots = 'slot=' + ','.join([str(i) for i in range(ranks[node]-nthread, ranks[node])])
        url = json_input['data_sources'][station][0].partition("mk5://")
        json_input['data_sources'][station][0] = url[1] + input_nodes[station] + "/" + url[2]
        ranks[node] -= nthread
        print >>fp, "rank", str(rank), "=", node, slots

    for i in range(8):
        for machine in machines:
          if ranks[machine] > 0:
            rank += 1
            print >>fp, "rank", str(rank), "=", machine, "slot=", str(ranks[machine]-1)
            ranks[machine] -= 1
    fp.close()
    output_control_file = "run_job.out.ctrl"
    outfp = open(output_control_file, "w")
    json.dump(json_input, outfp, indent=2)
    outfp.close()

    # Start the job.
    number_nodes = 3 + len(stations) + options.number_nodes
    sfxc = "`which sfxc`"
    cmd = "mpirun --mca btl_tcp_if_include " \
        + options.subnet + " " \
        + "--mca oob_tcp_if_include " \
        + options.subnet + " " \
        + "--mca orte_keep_fqdn_hostnames 1 " \
        + "--mca orte_hetero_nodes 1 "\
        + "-machinefile " + machine_file + " "\
        + "--rankfile " + rank_file + " " \
        + "-n " + str(number_nodes) + " " \
        + sfxc + " " + output_control_file + " " + vex_file \
        + " 2>&1 | tee " + log_file
    print cmd
    os.system(cmd)

    continue
