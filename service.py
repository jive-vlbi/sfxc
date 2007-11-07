#!/usr/bin/env python2.4
import sys
import re
import time
from ZSI.ServiceContainer import AsServer
from TranslationNode_services_server import *
from TranslationNode_mark5 import *
from TranslationNode_vex import *

inp =  open("mark5_connect_data.inp", "r")
portMark5Data = int(inp.readline())
portMark5Control = int(inp.readline())
ipMark = inp.readline()
fileName = inp.readline()
fileName = fileName.strip()
block_size = int(inp.readline())
portNumber = int(inp.readline())

class Service(TranslationNode):

	def soap_startTranslationJob(self, ps):
		rsp = TranslationNode.soap_startTranslationJob(self, ps)
		msg = self.request
		print 'Requested broker IP address: ', msg.Param0.BrokerIPAddress
		print 'Requested start time: ', msg.Param0.StartTime
		print 'Requested end time: ', msg.Param0.EndTime
		print 'Requested chunk size: ', msg.Param0.ChunkSize
		print 'Requested station name: ', msg.Param0.StationName
		print 'Requested experiment name: ', msg.Param0.ExperimentName

# this is where everything is calculated...

#		Mark5_connect(portMark5Data, portMark5Control, ipMark) 

# Information passed to us by the VLBI grid broker
		station = msg.Param0.StationName
		host = msg.Param0.BrokerIPAddress
		chunk_size = msg.Param0.ChunkSize
		job_start = parse_vex_time(msg.Param0.StartTime)
		job_end = parse_vex_time(msg.Param0.EndTime)
		vex_file_name = msg.Param0.ExperimentName
		experiment_name = vex_file_name.split('.')
		print "experiment_name ==>", experiment_name[0]
		
		print 'host IP address: ', host
		
		# Open VEX file
		vex = Vex(vex_file_name)

		chunk_start = job_start
		chunk_time = job_start
		start_scan = None

		sched = vex['SCHED']
		chunk_number = 0
		for scan in sched:
			if not start_scan:
				start_scan = scan
				pass
			scan_start = parse_vex_time(sched[scan]['start'])

			if scan_start > chunk_time:
				chunk_time = scan_start

			data_format = vex.get_data_format(scan,station)
			overhead = data_format_overhead[data_format]

# Find out the true length of the scan, which is just the maximum
# of the per-station length of the scan.
			secs = 0
			for info in sched[scan].getall('station'):
				secs = max(secs, info[2])
				continue
			scan_end = scan_start + float(secs.split()[0])

# If the this scan ends before the job starts, skip it.
			if scan_end < job_start:
				continue
			if scan_start > job_end:
				continue

# Calculate the data rate for this scan.
			mode = vex.get_mode(scan)
			bits_per_sample = vex.get_bits_per_sample(mode, station)
			num_channels = vex.get_num_channels(mode, station)
			sample_rate = vex.get_sample_rate(mode, station)
			data_rate = num_channels * sample_rate * bits_per_sample * overhead
			
			if (msg.Param0.ChunkSize == 0):
			  chunk_size=(scan_end - scan_start)*(data_rate/8)

			chunk_to_data_rate = int(chunk_size / (data_rate/8))
			if chunk_to_data_rate == 0:
				chunk_to_data_rate += 1
				
			chunk_size = chunk_to_data_rate * (data_rate/8)
			chunk_bytes = chunk_size

			start_position = float(vex.get_data_start(scan, station))
			start_position = int(start_position*1000*1000*1000)
			mark5_set_output = Mark5_set_position()
			time_real_start = mark5_set_output.bytes_starting_position(portMark5Data, portMark5Control, ipMark, start_position)
			time_real_start_ms = re.search("\.(\d{4})s",time_real_start)
			time_real_start_ms = float("0."+time_real_start_ms.group(1))
			time_real_start=re.sub("\.\d*","", time_real_start)
			time_real_start=parse_vex_time(time_real_start)
			if (job_start > scan_start):
				time_start_diff = time_real_start-job_start
			else:
				time_start_diff = time_real_start-scan_start
			bytes_starting_diff = (time_start_diff+time_real_start_ms)*(data_rate/8)

# Chop it up and chunks 
			chunk_end = chunk_time + chunk_size / (data_rate / 8)
			chunk_end = min(chunk_end, job_end)
			chunk_start_check = max(job_start, scan_start)
			
			chunk_real_end_size_temp = (scan_end-scan_start) * (data_rate / 8) / chunk_size
			chunk_real_end_size = (chunk_real_end_size_temp - int(chunk_real_end_size_temp))*chunk_size
			if chunk_real_end_size ==0:
				chunk_real_end_size = chunk_size
			scan_size = (scan_end-scan_start)*(data_rate/8)
			
			while chunk_end <= scan_end:
				print "chunk", chunk_number, scan, \
					strftime("%Yy%jd%Hh%Mm%Ss", localtime(chunk_start)), scan, \
					strftime("%Yy%jd%Hh%Mm%Ss", localtime(chunk_end))
					
				if (time_real_start > chunk_start_check and int(bytes_starting_diff/chunk_size)>0):
					chunk_real_size=0
				elif (time_real_start >= chunk_start_check and int(bytes_starting_diff/chunk_size)==0):
					chunk_real_size=chunk_size - time_real_start_ms * (data_rate / 8)
				elif (chunk_end == scan_end):
					chunk_real_size = chunk_real_end_size
				elif ((chunk_end-chunk_start)*(data_rate/8) < chunk_size):
					chunk_real_size=(chunk_end-chunk_start)*(data_rate/8)
				else :
					chunk_real_size=chunk_size

				chunk_diff=chunk_end-chunk_start
				chunk_diff_size = chunk_diff*(data_rate/8)

				chunk_start_check=chunk_end

				sendFile = experiment_name[0] + '_' + station + '_' + str(scan) + "_" + str(chunk_number) + '.m5a'
				sendFile = fileName + sendFile.lower()
				mark5_chunk_output = Mark5_get_chunks(portMark5Data, portMark5Control, ipMark, sendFile, block_size, chunk_real_size, start_position)
				print mark5_chunk_output
				
				time.sleep(1)

				if chunk_end >= job_end:
					break

				if chunk_end >= scan_end:
					break

				chunk_start = chunk_time = chunk_end
				chunk_end = chunk_time + chunk_size / (data_rate / 8)
				if scan_end < job_end:
					chunk_end = min(chunk_end, scan_end)
				elif scan_end >= job_end:
					chunk_end = min(chunk_end, job_end)

				chunk_bytes = chunk_size
				chunk_number += 1
				start_scan = scan
				continue

			chunk_time = scan_end
			chunk_start = chunk_time
			chunk_bytes = chunk_size
			chunk_number += 1
			start_scan = scan
			chunk_bytes -= (scan_end - scan_start) * (data_rate / 8)
			chunk_number = 0
			continue



# end of calculation
#		Mark5_disconnect()
		return rsp

if __name__ == "__main__" :
	port = portNumber
	AsServer(port, (Service('test'),))