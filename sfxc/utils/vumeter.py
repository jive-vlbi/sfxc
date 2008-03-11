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
corrnode = 0
runtimefull = 0
start_time2 = 0

def update_state(id, curr, max):
	global process, corrnode
	if process.has_key(id):
		process[id][0] = curr
		process[id][1] = max
	else:
		process[id] = [1,1,0, "",0,0]		 
		#corrnode=corrnode+1	

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
	global process, funfunid, runtimefull
		
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

	if not runtimefull == 0:
		display+="  Real-time full:"+`(current_time-start_time2)*100/runtimefull`+"%"
	

	display+="\n"
	idx = 0
	if len(process.keys()) < 25:
		
		for i in process.keys():
			if i < 10:
				display+="Core["+`i`+"]:  P: "+process[i][3]+"["+`process[i][4]`+"]"
			else:
				display+="Core["+`i`+"]: P: "+process[i][3]+"["+`process[i][4]`+"]"
			display+=bargraph(process[i][0], process[i][1], 40)
			display+=":"+`process[i][2]`+"\n"
	elif len(process.keys()) < 50: 
		for i in process.keys():
			idx=idx+1
			if i < 10:
				display+="Core["+`i`+"]:  P["+`process[i][4]`+"]"
			else:
				display+="Core["+`i`+"]: P["+`process[i][4]`+"]"
			display+=bargraph(process[i][0], process[i][1], 25)
 			display+=":"+`process[i][2]`
			if (idx+1) % 2 == 0:
				display+="\n"
			else:
				display+="   "
	elif len(process.keys()) < 100: 
		for i in process.keys():
			idx = idx+1
			if i < 10:
				display+="Core["+`i`+"]: "
			else:
				display+="Core["+`i`+"]:"
			display+=bargraph(process[i][0], process[i][1], 10)
 			display+=":"+`process[i][2]`
			if (idx) % 4 == 0:
				display+="\n"
			else:
				display+="   "
			
	display+="\n"
	display+="-----------------------------------------------------------------------------------------------\n"
	for i in last_lines:
		display+=i
	display+="-----------------------------------------------------------------------------------------------"
	return display


def display(runtime, last_lines):
	global process, runtimefull,corrnode, start_time2, current_time
	displaystr = make_display(runtime, last_lines)
	print displaystr
	

last_lines=[]
total_time = time.time()
total_time2 = 0
for i in range(0,10000):
        iter_time = time.time()
	process={}
	corrnode = 0
	current_time = 0
	tmpfile = open("std_output.txt", "r")
	for line in tmpfile:
		ret = re.match(".* PROGRESS t=[0-9]*:.*start_time: ([0-9a-z]*)",line)
		if ret:
			set_start_time(ret.groups()[0])
		ret = re.match(".* PROGRESS t=[0-9]*:.*stop_time: ([0-9a-z]*)",line)
		if ret:
			set_stop_time(ret.groups()[0])
		
		ret = re.match("#([0-9]*) correlation_core.cc, [0-9]+:  PROGRESS t=[0-9]*: node ([0-9]*), ([0-9]*) of ([0-9]*)", line)
		if ret:
			update_state(int(ret.groups()[1]), int(ret.groups()[2]), int(ret.groups()[3]))
		else:
			ret = re.match(".* PROGRESS t=[0-9]*:.* start ([0-9a-z]*), channel ([0-9,]*) to correlation node ([0-9]+)", line)			
			if ret:
				update_count(int(ret.groups()[2]), ret.groups()[1], ret.groups()[0])
			
			else:
				ret = re.match(".* PROGRESS t=[0-9]*:.* time ([0-9]+)", line)			
				if ret:
					#print "Hello world:"+ret.groups()[0]
					update_current( int(ret.groups()[0]) )

				else:
					last_lines.append(line)
					if( len(last_lines) > 6 ):
						last_lines.pop(0)
	
	tmpfile.close()

	time.sleep(1);

	#if not(corrnode == 0) and len(process.keys()) == corrnode:
	#	if total_time2 == 0:
	#		total_time2 = int(time.time())
	#		start_time2 = current_time
	#	runtimefull = int(time.time()-total_time2)

	display(int(time.time()-total_time), last_lines)
	#print "HELLO BISON:"+`total_time2`+" " + `start_time2` 
		
