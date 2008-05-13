#! /usr/bin/python

import sys, os, popen2, re
import time
import tempfile, fileutils
import numarray as na
from pylab import *

inputfilename = None
#inputfilename="std_output.txt"

if len(sys.argv) == 2:
  inputfilename = sys.argv[1]

process = {}
start_time = 0
stop_time = 1
current_time = 0
corrnode = 0
runtimefull = 0
start_time2 = 0

# sfxc_status can be:
#   stopped: 0 
#   initing: 1
#   running: 2
#   finalizing: 3
sfxc_status = 0

rt_x0=0
rt_y0=0
rt_xs = []
rt_ys = []
lastslice = -1

def update_state(id, curr, max):
  global process, corrnode
  if process.has_key(id):
    process[id][0] = curr
    process[id][1] = max
  else:
    process[id] = [1,1,0, "",0,0]     
    #corrnode=corrnode+1  

def count_packet():
  global process
  counter = 0
  for i in process.keys():
    counter+=process[i][2]
  return counter

def update_count(id, packet, time):
  global process, corrnode
  if process.has_key(id):
    if process[id][3] == "" and process[id][5] == 0:
      corrnode=corrnode+1
      process[id][5] = 1  
    process[id][2] = process[id][2]+1
    process[id][3] = time
    process[id][4] = packet
  else:
    process[id] = [0,1,0, "",0,0]
    #corrnode=corrnode+1

def time_to_int(time):
  ret = re.match("([0-9]*)y([0-9]*)d([0-9]*)h([0-9]*)m([0-9]*)s(([0-9]*)ms)?", time)
  #print time,
  itime = -1
  if ret:
                itime = int(ret.groups()[2])
                itime = int(ret.groups()[3]) + 60*itime
                itime = int(ret.groups()[4]) + 60*itime
                itime = 1000*itime
                if (ret.groups()[6]):
                        itime = itime + int(ret.groups()[6])

  #itime = itime*1000000
  #print " this give: "+`itime`
  return itime
  
def set_start_time(time):
  global start_time
  global current_time
  start_time = time_to_int(time)
  current_time = start_time
  print "SET STARTIME TO:"+`start_time`

def set_stop_time(time):
  global stop_time
  stop_time = time_to_int(time)-1000
  print "SET STOPTIME TO:"+`stop_time`

def update_current(ctime):
  global current_time
  new_time = time_to_int(ctime)
  #print "ToTO:"+ctime
  if new_time > current_time:
    current_time = new_time

#funfun = ['-','\\','|', '/']
funfun = ['>']
funfunid = 0  
def bargraph(current, end, size):
  global funfun, funfunid
  current_pos = 0
  if end != 0:
    current_pos=(current*size)/end
  string="["
  if current_pos > size: current_pos = size-1
  for j in range(0, current_pos):
    string+="#"
  string+=funfun[funfunid]
  for j in range(current_pos, size):
    string+=" "
  return string+"]"

def percent(current, total):
  return ""+`int(current*100/total)`+"%"


def write_xp_details(fp):
  global start_time, stop_time, current_time, sfxc_status
  output=' "experiment"  : { \n'
  output+='\t "status" : '+`sfxc_status`+', \n'
  output+='\t "lastslice"     : '+`lastslice`+', \n' 
  output+='\t "duration"     : '+`(stop_time-start_time)/1000`+', \n'  
  output+='\t "start_time"    : '+`start_time`+', \n'  
  output+='\t "stop_time"     : '+`stop_time`+', \n'
  output+='\t "current_time"  : '+`current_time`+' \n'   
  output+='}'
  
  fp.write(output)
  
def write_correlationnode(fp):
  output=' "correlationnode" : [ \n'
  
  first = True
  for i in process.keys():
    if not first:
      output+=', \n'
    first = False  
    output+="{ \n"
    output+='\t"name": "correlationcore'+`i`+'", \n'  
    output+='\t"load": '+`20`+', \n'
    output+='\t"current_fft": '+`process[i][0]`+', \n'
    output+='\t"total_fft": '+`process[i][1]`+', \n'
    output+='\t"ts_count": '+`process[i][2]`+' \n'
    output+="} "  
  
  output+=' \n]\n '
  fp.write(output)    

def write_json(outfile):
  fp = open(outfile, "w")
  
  #write_inputnodefp)
  fp.write("{")
  write_xp_details(fp)
  fp.write(", ")
  write_correlationnode(fp)   
  fp.write("}")
  
  fp.close()

def write_rt_picture(outfile, xs, ys):
  print "Gen picture....:"+outfile
  p = figure(figsize=[10, 10])
  
  title("Real-time vs Computation time")
  xlabel("Computation time (in seconds)")
  ylabel("Real time (in seconds)")
  
  plot(xs, ys, color='b', label="sfxc runtime")
  plot(xs, xs, color='r', label="ideal real-time application")
  legend(loc=2) 
    
  axis([0, 400, 0, 400])
  savefig(outfile, dpi=50) 

last_lines=[]
time_start_correlation = 0
time_end_correlation = 0
total_time2 = 0
found_start_correlation = False
counter = 0

start_prev_timeslice = 0
start_curr_timeslice = 0
found_start_correlation = False
iter_time = time.time()
process={}
corrnode = 0
current_time = 0

time_start_correlation = 0
time_end_correlation = 0


fp = fileutils.Incr_file(inputfilename)
for line in fp.read():
    #print "LINE:" +line
    counter+=1
    
    if re.search("STARTING:",line):
      print "========= >STARTINNN!"
      sfxc_status = 1
    if re.search("START_RESERVATION:",line):
      sfxc_status = 1
      print "========= >INIT!"+`sfxc_status`
    if re.search("START_SFXC:",line):
      sfxc_status = 2
      print "========= >SFXC   !"+`sfxc_status`
    if re.search("STOP_SFXC:", line):
      sfxc_status = 0
      print "========= >STOPPING!"+`sfxc_status`
                 
          
    ret = re.match(".* PROGRESS t=[0-9]*:.*start_time: ([0-9a-z]*)",line)
    if ret:
      set_start_time(ret.groups()[0])
    if not ret:
      ret = re.match(".* PROGRESS t=[0-9]*:.*stop_time: ([0-9a-z]*)",line)
      if ret:
        set_stop_time(ret.groups()[0])
    if not ret:
      ret = re.match("#([0-9]*) correlation_core.cc, [0-9]+:  PROGRESS t=[0-9]*: node ([0-9]*), ([0-9]*) of ([0-9]*)", line)
      if ret:
        try:
          update_state(int(ret.groups()[1]), int(ret.groups()[2]), int(ret.groups()[3]))
        except:
          print "BROKEN LINE:" + line
          print "     AND:" + `ret.groups()`

    if not ret:
      ret = re.match(".* PROGRESS t=[0-9]*:.* start ([0-9a-z]*), channel ([0-9,]*) to correlation node ([0-9]+)", line)      
      if ret:
        update_count(int(ret.groups()[2]), ret.groups()[1], ret.groups()[0])
        update_current( ret.groups()[0] )
    if not ret:
      ret = re.match(".* PROGRESS t=([0-9]*): start correlating", line)      
      if ret:
        if time_start_correlation == 0:
          time_start_correlation = int(ret.groups()[0])/1000000
        found_start_correlation = True
    if not ret:
      ret = re.match(".* PROGRESS t=([0-9]*): terminating nodes", line)      
      if ret:
        if time_end_correlation == 0:
          time_end_correlation = int(ret.groups()[0])/1000000
    if not ret:
      ret = re.match(".* PROGRESS t=([0-9]*): starting timeslice ([0-9]*)", line)      
      if ret:
        start_prev_timeslice = start_curr_timeslice
        start_curr_timeslice = int(ret.groups()[0])
        
        lastslice+=1        
        x = int(ret.groups()[0])/1000000
        y = int(ret.groups()[1])/1000.
    
        if rt_x0 == 0:
          rt_x0 = x
          rt_y0 = y
      
        rt_xs.append(x-rt_x0),
        rt_ys.append(y-rt_y0)
        write_rt_picture("site/cached/slices"+`lastslice`+".png", rt_xs, rt_ys)

    write_json("tmpdata/status_file_json.txt")  
    if sfxc_status == 0 : 
      sys.exit(0)
  
