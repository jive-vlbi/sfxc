#! /usr/bin/python

#
# An VU-meter to show the progression of the
# sfxc computation in text-mode real-time
# This utils was made with all the love
# it deserve.
#
import sys, os, popen2, re, simplejson
import time
import tempfile

process = {}
start_time = 0
stop_time = 1
current_time = 0

def update_state(id, curr, max):
	global process
	if process.has_key(id):
		process[id][0] = curr
		process[id][1] = max
	else:
		process[id] = [1,1,0, "",0]		 

def update_count(id, packet, time):
	global process
	if process.has_key(id):
		process[id][2] = process[id][2]+1
		process[id][3] = time
		process[id][4] = packet
	else:
		process[id] = [0,1,0, "",0]

def time_to_int(time):
	ret = re.match("([0-9]*)y([0-9]*)d([0-9]*)h([0-9]*)m([0-9]*)s", time)
	#print time,
	itime = -1
	if ret:
		itime = (int(ret.groups()[4])+ 60*( int(ret.groups()[3]) + 60*( int(ret.groups()[2]) + 0*( int(ret.groups()[1]) + 0*int(ret.groups()[0]) ))))
	#itime = itime*1000000
	#print " this give: "+`itime`
	return itime
	
def set_start_time(time):
	global start_time
	global current_time
	start_time = time_to_int(time)
	current_time = start_time

def set_stop_time(time):
	global stop_time
	stop_time = time_to_int(time)

def update_current(ctime):
	global current_time
	if ctime > current_time:
		current_time = ctime/1000000

#funfun = ['-','\\','|', '/']
funfun = ['>']
funfunid = 0	
def bargraph(current, end, size):
	global funfun, funfunid
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

def make_display(runtime, last_lines):
	global process, funfunid
		
	funfunid+=funfunid+1
	if funfunid >= len(funfun): 
		funfunid = 0
	
	display="\x1b[2J"
	display+="-----------------------------========= SFXC MONITOR ========-----------------------------------\n"
	display+="running since: "+`runtime`+"sec\n" 
	display+="Begin time:"+`start_time`+" End time:"+`stop_time`+" Current time:"+`current_time`+"\n"
	display+="Progress: "+bargraph(current_time-start_time, stop_time-start_time, 50)+": "+percent(current_time-start_time, stop_time-start_time)
	if not runtime == 0:
		display+="  Real-time ratio:"+`(current_time-start_time)*100/runtime`+"%"
	display+="\n"

	for i in process.keys():
		if i < 10:
			display+="Correlation core["+`i`+"]:  processing: "+process[i][3]+"["+`process[i][4]`+"]"
		else:
			display+="Correlation core["+`i`+"]: processing: "+process[i][3]+"["+`process[i][4]`+"]"
		 
		display+=bargraph(process[i][0], process[i][1], 40)
		display+=":"+`process[i][2]`+"\n"
	display+="\n"
	display+="-----------------------------------------------------------------------------------------------\n"
	for i in last_lines:
		display+=i
	display+="-----------------------------------------------------------------------------------------------"
	return display


def display(runtime, last_lines):
	global process
	displaystr = make_display(runtime, last_lines)
	print displaystr
	

last_lines=[]
total_time = time.time()
for i in range(0,10000):
        iter_time = time.time()
	process={}
	tmpfile = open("std_output.txt", "r")
	for line in tmpfile:
		ret = re.match(".*start_time: ([0-9a-z]*)",line)
		if ret:
			set_start_time(ret.groups()[0])
		ret = re.match(".*stop_time: ([0-9a-z]*)",line)
		if ret:
			set_stop_time(ret.groups()[0])
		
		ret = re.match("#([0-9]*) correlation_core.cc, [0-9]+: PROGRESS node ([0-9]*), ([0-9]*) of ([0-9]*)", line)
		if ret:
			update_state(int(ret.groups()[1]), int(ret.groups()[2]), int(ret.groups()[3]))
		else:
			ret = re.match(".* start ([0-9a-z]*), channel ([0-9]*) to correlation node ([0-9]+)", line)			
			if ret:
				update_count(int(ret.groups()[2]), int(ret.groups()[1]), ret.groups()[0])
			
			else:
				ret = re.match(".* time ([0-9]*)", line)			
				if ret:
					update_current( int(ret.groups()[0]) )

				else:
					last_lines.append(line)
					if( len(last_lines) > 6 ):
						last_lines.pop(0)
	
	tmpfile.close()
	while (time.time()-iter_time) < 0.1:
		None
	display(int(time.time()-total_time), last_lines)
		
