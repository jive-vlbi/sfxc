#!/usr/bin/env python
import json
import os
import re
import optparse
import socket
import subprocess
import sys
import time
import urlparse
import imp

urlparse.uses_relative.append('mk5')
urlparse.uses_netloc.append('mk5')
urlparse.uses_params.append('mk5')
urlparse.clear_cache()

base_local_cmd_port = 2620
base_data_port = 2630

input_ip_map = {
  "e0" : '192.42.120.82', 'e1' : '192.42.120.86', 'f0' : '192.42.120.83', 'f1' : '192.42.120.87',
  'g0' : '192.42.120.84', 'g1' : '192.42.120.88', 'h0' : '192.42.120.85', 'h1' : '192.42.120.89'
}

# Proper time.
os.environ['TZ'] = "UTC"

def vex2time(str):
    tupletime = time.strptime(str, "%Yy%jd%Hh%Mm%Ss");
    return time.mktime(tupletime)

def time2vex(secs):
    tupletime = time.gmtime(secs)
    return time.strftime("%Yy%jd%Hh%Mm%Ss", tupletime)

def start_reader(station, node, local_cmd_port):
    args = ["ssh", node, "/home/sfxc/jive5ab_runjob/jive5ab", "-m3", "-p", `local_cmd_port`]
    print "starting", station, ":", args
    log = open(station + "-reader.log", 'w')
    p = subprocess.Popen(args, stdout=log, stderr=log)
    return p

def stop_readers(json_input, input_map, readers, nstreams):
  for stream in range(nstreams):
    for station, station_info in input_map.iteritems():
      if stream < json_input["number_streams"][station]:
        cmd_port = station_info["local_cmd_port"][stream]
        node = station_info["node"][stream]
        finalize_flow(node, cmd_port)

    time.sleep(5)
   
    for station, station_info in input_map.iteritems():
      if stream < json_input["number_streams"][station]:
        node = "sfxc-%s.sfxc"%(station_info["node"][stream])
        args = ["ssh", node, "pkill", "jive5ab"]
        subprocess.call(args)

  # Wait for the readers to stop
  for stream in range(nstreams):
    for station, station_info in input_map.iteritems():
      if (stream < json_input["number_streams"][station]) and (stream < len(readers)):
        try:
          readers[stream][station].wait()
        except KeyError:
          pass

def send_local_command(node, cmd_port, command):
  try:
      s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      s.settimeout(5)
      print "node = ", node
      s.connect((node, cmd_port))
      print command
      s.send(command)
      data = s.recv(1024)
      print repr(data)
      s.close()
  except:
      pass

def setup_flow(node, cmd_port, data_port, data_socket):
  # Before seting up the data flow, first make sure connection is closed
  finalize_flow(node, cmd_port)

  send_local_command(node, cmd_port, "mode=legacyvdif:8:8192")
  send_local_command(node, cmd_port, "net_protocol=udp:1342177728:524288;")
  send_local_command(node, cmd_port, "net_port=%d;" % data_port)
  send_local_command(node, cmd_port, "mtu=9000;")
  send_local_command(node, cmd_port, "net2sfxc=open:%s;" % data_socket)
  #send_local_command(station, "net2file=open:/tmp/%s,w;" % station)
  #send_local_command(station, "net2file=open:/dev/null,w;")

def finalize_flow(node, cmd_port):
  send_local_command(node, cmd_port, "net2sfxc=close;")
  #send_local_command(node, cmd_port, "net2file=close;")

def setup_readers(json_input, input_map, nstreams):
# Start the data readers and configure them
  reader_list = []
  for stream in range(nstreams):
    readers = {}
    for station, station_info in input_map.iteritems():
      if stream < json_input["number_streams"][station]:
        node = "sfxc-%s.sfxc"%(station_info["node"][stream])
        cmd_port = station_info["local_cmd_port"][stream]
        readers[station] = start_reader(station, node, cmd_port)

    time.sleep(5)

    for station, station_info in input_map.iteritems():
      if stream < json_input["number_streams"][station]:
        node = "sfxc-%s.sfxc"%(station_info["node"][stream])
        cmd_port = station_info["local_cmd_port"][stream]
        data_port = station_info["data_port"][stream]
        data_socket = station_info["data_socket"][stream]

        setup_flow(node, cmd_port, data_port, data_socket)
    reader_list.append(readers)
  return reader_list

def create_machine_files(json_input, input_map, nstreams): 
# Generate machine files and rank files
  stations = json_input["stations"]
  exper_name = json_input["exper_name"]
  machines = [['a','b','c','d'], ['e', 'f', 'g', 'h'], ['i','j']]
  rankfiles = []
  machinefiles = []
  for i in range(nstreams):
    # Create a list of available nodes
    nodes = {}
    nslots = 0
    for j in range(4):
      node = "%c%d"%(machines[0][i], j)
      nodes[node] = 8
      node = "%c%d"%(machines[1][i], j)
      nodes[node] = 8
      nslots += 16
    node = "%c%d"%(machines[2][0], i)
    nodes[node] = 16
    node = "%c%d"%(machines[2][1], i)
    nodes[node] = 16
    nslots += 32

    # create machine file
    filename = "%s-%d.machines"%(exper_name, i+1)
    machinefiles.append(filename)
    outfile = open(filename, 'w')
    for node, slots in nodes.iteritems():
      outfile.write("sfxc-%s.sfxc slots=%d\n"%(node, slots))
    outfile.close()
        
    # create rankfile
    filename = "%s-%d.ranks"%(exper_name, i+1)
    rankfiles.append(filename)
    outfile = open(filename, 'w')
    # manager, log, and output node
    manager_node = "%c0"%machines[0][i]
    log_node = "%c0"%machines[0][i]
    output_node = "%c0"%machines[0][i]
    outfile.write("rank 0=sfxc-%s.sfxc slot=%d\n"%(manager_node, nodes[manager_node]-1))
    nodes[manager_node] -= 1
    outfile.write("rank 1=sfxc-%s.sfxc slot=%d\n"%(log_node, nodes[log_node]-1))
    nodes[log_node] -= 1
    outfile.write("rank 2=sfxc-%s.sfxc slot=%d,%d\n"%(output_node, nodes[output_node]-2, nodes[output_node]-1))
    nodes[output_node] -= 2
    nslots -= 4
    # Input nodes
    rank = 3
    for station in stations:
      if json_input["number_streams"][station] > i:
        node = input_map[station]["node"][i]
        outfile.write("rank %d=sfxc-%s.sfxc slot=%d,%d,%d,%d\n"%(rank,node,nodes[node]-4,nodes[node]-3,nodes[node]-2,nodes[node]-1))
        nodes[node] -= 4
        nslots -= 4
        rank += 1
    # Correlator nodes
    while nslots > 0:
      for node, slots in nodes.iteritems():
        if slots > 0:
          outfile.write("rank %d=sfxc-%s.sfxc slot=%d\n"%(rank, node, slots-1))
          nodes[node] -= 1
          nslots -= 1
          rank += 1
  return machinefiles, rankfiles

def create_input_mapping(json_input, nstreams):
  input_mapping = {}
  stations = json_input["stations"]
  nodes = ['e', 'f', 'g', 'h']
  for i in range(nstreams):
    node = nodes[i]
    for j, station in enumerate(stations):
      if i >= json_input["number_streams"][station]:
        continue
      if i == 0:
        input_mapping[station] = {"node":[], "local_cmd_port":[], "data_port":[], "data_socket":[]}
      input_mapping[station]["node"].append(node+`j%2`)
      input_mapping[station]["local_cmd_port"].append(base_local_cmd_port + j/2)
      input_mapping[station]["data_port"].append(base_data_port + j/2)
      url = urlparse.urlparse(json_input["data_sources"][station][0]) 
      data_socket = os.path.dirname(url.path)
      input_mapping[station]["data_socket"].append(data_socket)
  return input_mapping      

def create_control_files(json_input, nstreams):
  # For each input stream create a control file
  ctrl_files = []
  exper_name = json_input["exper_name"]
  stations = json_input["stations"]
  for nr in range(nstreams):
    filename = "%s-%d.ctrl"%(exper_name, nr+1)
    ctrl_files.append(filename)
    outfile = open(filename, "w")
    new_json = dict((key, val) for (key,val) in json_input.items())
    channels = []
    for i in range(nr*8+1,(nr+1)*8+1):
      channels.append("CH%02d"%i)
    new_json["channels"] = channels
    output_file ="file://%s/%s-%d.cor"%(os.getcwd(), exper_name, nr+1)
    new_json["output_file"] = output_file
    new_stations = []
    for station in stations:
      if json_input["number_streams"][station] > nr:
        new_stations.append(station)
    new_json["stations"] = new_stations
    json.dump(new_json, outfile, indent=2)
  return ctrl_files

def create_vex_files(input_vex, json_input, nstreams):
  # For each input stream create a vex file
  exper_name = json_input["exper_name"]
  stations = json_input["stations"]
  vex_files = []
  streams = []
  # Determine which stations there are in each stream, and create vex files
  for nr in range(nstreams):
    filename = "%s-%d.vix"%(exper_name, nr+1)
    vex_files.append(filename)
    outfile = open(filename, "w")
    new_stations = []
    for station in stations:
      if json_input["number_streams"][station] > nr:
        new_stations.append(station)
    streams.append([outfile,new_stations])
  # Create vex files 
  in_mode_block = False
  infile = open(input_vex, 'r')
  line = infile.readline()
  while line != "":
    if in_mode_block:
      if first:
        first = False
        # create modes
        for i in range(len(streams)):
          modes = {}
          for station in streams[i][1]:
            mode = json_input["vex_threads"][station][i]
            try:
              modes[mode].append(station)
            except KeyError:
              modes[mode] = [station]
          for mode,list in modes.iteritems():
            streams[i][0].write("   ref $THREADS = %s:%s;\n"%(mode,":".join(list)))
      if line.lstrip().startswith("def "):
        first = True
      elif line.lstrip().startswith("enddef;"):
        in_mode_block = False
      elif "$THREADS" in line.split():
        line = infile.readline()
        continue
    elif line.lstrip().startswith("$MODE;"):
      first = False
      in_mode_block = True
    for stream in streams:
      stream[0].write(line)
    line = infile.readline()
  return vex_files
#
#################################### MAIN ################################################
#
usage = "usage: %prog [options] vexfile ctrlfile"
parser = optparse.OptionParser(usage=usage)
parser.add_option("-n", "--nstreams", dest="max_streams",
                  default=4, type="int",
                  help="Maximum number of 1 Gb/s streams per station")
parser.add_option("-s", "--stations", dest="stations",
                  type="string", help="Stations to correlate", metavar="LIST")
parser.add_option("-c", "--configure-harrobox", dest="cfg_harrobox",
                  type="string", help="Python module which generates harrobox configuration commands")

(options, args) = parser.parse_args()
if len(args) < 2:
  parser.error("incorrect number of arguments")

vex_file = args[0]
ctrl_file = args[1]

fp = open(ctrl_file, 'r')
json_input = json.load(fp)
fp.close()
exper = json_input["exper_name"]

# The maximum 1 Gb/s to be started for each station
nstreams = json_input["number_streams"]

# Get sorted list of stations
if options.stations == None:
  stations = json_input["stations"]
else:
  stations = options.stations.split(',')
stations.sort()
json_input["stations"] = stations

#Determine the maximum number of 1 Gb/s streams available to any station
max_streams = 0
for station in stations:
  if max_streams < nstreams[station]:
    max_streams = nstreams[station]
nstreams = min(max_streams, options.max_streams)

# Generate list of data sources
data_sources = {}
for station in stations:
  data_sources[station]=["mk5:///tmp/mk5-%s/NONE:0"%station]
json_input["data_sources"] = data_sources
try:
    input_map={}
    readers=[]
    # Make sure the delay files are up to date, and generate new ones
    # if they're not.
    procs = {}
    success = True
    for station in stations:
        path = urlparse.urlparse(json_input['delay_directory']).path
        delay_file = path + '/' +  exper + '_' + station + '.del'
        if not os.access(delay_file, os.R_OK) or \
                os.stat(delay_file).st_mtime < os.stat(vex_file).st_mtime:
            print 'Creating', delay_file
            args = ['generate_delay_model', vex_file, station, delay_file]
            procs[station] = subprocess.Popen(args, stdout=subprocess.PIPE)
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
    if not success:
        raise Hell
       
    # Generate control files
    ctrlfiles = create_control_files(json_input, nstreams) 

    # Generate vexfiles
    vexfiles = create_vex_files(vex_file, json_input, nstreams)

    # generate mapping of the input nodes
    input_map = create_input_mapping(json_input, nstreams)

    # Generate machine files and rank files
    machinefiles, rankfiles = create_machine_files(json_input, input_map, nstreams) 

    # Configure the data readers
    readers = setup_readers(json_input, input_map, nstreams) 

    # Start the job.
    procs = []
    for i in range(nstreams):
      #sfxc_gui = '/home/sfxc/bin/sfxc-gui.py'
      sfxc_gui = '/home/sfxc/closure_phase_plot/run-gui.sh'
      args = [sfxc_gui, '-s', '-e', vexfiles[i], ctrlfiles[i], machinefiles[i], rankfiles[i]]
      log_file = "%s.log-%d"%(exper, i+1)
      log = open(log_file, 'w')
      p = subprocess.Popen(" ".join(args), stdout=log, stderr=log, shell=True)
      procs.append(p)

    if options.cfg_harrobox != None:
      raw_input("Press enter to start transfers : ")
      # create dict : Station : [(stream_nr, input_node_ip, data_port), ...]
      m = {}
      for key,values in input_map.iteritems():
        m[key] = []
        for i in range(len(values["node"])):
          node = values["node"][i]
          port = values["data_port"][i]
          m[key].append((i, input_ip_map[node], port))
      # Generate list of commands for the Harrobox
      hb = imp.load_source("hbcommands", options.cfg_harrobox)
      cmds = hb.hbcommands(m)
      cmds_name = ".hb-commands.txt"
      outfile = open(cmds_name, "w")
      for cmd in cmds:
        outfile.write(cmd+'\n')
      outfile.close()
      # Send commands to ccsops and execute them with vlbish
      args = ['scp', cmds_name, 'jops@ccsops:']
      subprocess.Popen(args).wait()
      args = ['ssh', 'jops@ccsops', 'vlbish', '"play %s"'%cmds_name]
      subprocess.Popen(args).wait()
    time.sleep(20)
    for p in procs:
      p.wait()
finally:
    stop_readers(json_input, input_map, readers, nstreams)
