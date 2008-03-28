#!/usr/bin/env python2.4
# Author: Huseyin Ozdemir
# Date: 20080328
#
# TODO: A proper kill method to stop the service and disconnect from Mark5 properly
#
import sys
import time
import os
from os.path import join, getsize, exists, split
from ZSI.ServiceContainer import AsServer
from ZSI import ServiceProxy
from TranslationNode_services_server import *
from TranslationNode_mark5 import *
from TranslationNode_vex import *
from TranslationNodeNotification import *
# we need to skip comments in input file.
# For that we make use of re
import re

# define comment characters
comment_char = re.compile(r'^\s*#')

# define a list and initialize it with anything and don't use inp[0] later on 
inp = ['##']
# if the first character matches the comment character skip that line
# otherwise read the line
for line in file('mark5_connect_data.inp'):
  if comment_char.match(line):
    continue
  inp.append(line)

# read in the Data port number
portMark5Data = int(inp[1].strip())

# read in the Control port number
portMark5Control = int(inp[2].strip())

# IP address of the Mark5 that will be connected
ipMark = inp[3].strip()

# IP address of the host where the service is run
host = inp[4].strip()

# url address of the grid ftp 
#including the location of the directory where the files to be copied
gridFtpIP = inp[5].strip()

# file path that the data will be copied to
fileName = inp[6].strip()
fileName = fileName.strip()

# read in the block size
block_size = int(inp[7].strip())

# port number 
portNumber = int(inp[8].strip())

class Service(TranslationNode):

  def soap_startTranslationJob(self, ps):
    """ Main service function actually starts the translation job."""
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
    brokerIP = msg.Param0.BrokerIPAddress
    chunk_size = msg.Param0.ChunkSize
    job_start = parse_vex_time(msg.Param0.StartTime)
    job_end = parse_vex_time(msg.Param0.EndTime)
    vex_file_name = msg.Param0.ExperimentName
    vex_file_name = vex_file_name.strip()
    experiment_name = vex_file_name.split('.')
#		print "experiment_name ==> '"+str(type(vex_file_name))-+"'"
    
    print 'host IP address: ', host
    print 'broker address: ', brokerIP
    
    # Open VEX file
    vex = Vex(str(vex_file_name))

    chunk_start = job_start
    chunk_time = job_start
    start_scan = None

    sched = vex['SCHED']
    chunk_number = 0
# This is the main loop
    for scan in sched:
      if not start_scan:
        start_scan = scan
        pass
      scan_start = parse_vex_time(sched[scan]['start'])

      if scan_start > chunk_time:
        chunk_time = scan_start

      data_format = vex.get_data_format(scan, station)
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
# Calculated the size of a chunk that fits in to 1 second
# make sure that the chunk size is at least equals to 1 second data size
# if required chunk size corresponds to a floating point seconds use int value
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

# set head position on the disk on Mark5
      start_position = float(vex.get_data_start(scan, station))
      start_position = int(start_position*1000*1000*1000)
      mark5_set_output = Mark5_set_position()
      time_real_start = mark5_set_output.bytes_starting_position(host, portMark5Data, portMark5Control, ipMark, start_position)
      time_real_start_ms = re.search("\.(\d{4})s", time_real_start)
      time_real_start_ms = float("0."+time_real_start_ms.group(1))
      time_real_start=re.sub("\.\d*", "", time_real_start)
      time_real_start=parse_vex_time(time_real_start)
      if (job_start > scan_start):
        time_start_diff = time_real_start-job_start
      else:
        time_start_diff = time_real_start-scan_start
      bytes_starting_diff = (time_start_diff+time_real_start_ms)*(data_rate/8)

# Chop it up in to chunks 
      chunk_end = chunk_time + chunk_size / (data_rate / 8)
      chunk_end = min(chunk_end, job_end)
      chunk_start_check = max(job_start, scan_start)

      chunk_real_end_size_temp = (scan_end-scan_start) * (data_rate / 8) / chunk_size
      chunk_real_end_size = (chunk_real_end_size_temp - int(chunk_real_end_size_temp))*chunk_size
      if chunk_real_end_size ==0:
        chunk_real_end_size = chunk_size
      scan_size = (scan_end-scan_start)*(data_rate/8)

# This is the main while loop
      while chunk_end <= scan_end:
# print out some info about the current chunk
        print "chunk", chunk_number, scan, \
          strftime("%Yy%jd%Hh%Mm%Ss", localtime(chunk_start)), scan, \
          strftime("%Yy%jd%Hh%Mm%Ss", localtime(chunk_end))

# Consider several conditions to deliver the right chunk
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

# give a logical name to the file that is downloaded from Mark5
        sendFile = experiment_name[0] + '_' + station + '_' + str(scan) + "_" + str(chunk_number) + '.m5a'
        sendFile = fileName + sendFile.lower()
        print "file to be sent: " + sendFile

#  If file is already downloaded from Mark5 before, we don't need to download it again
#  just split the existing file in to required chunk size and send it
        if exists(sendFile):
          file_size =  getsize(sendFile)

        if (exists(sendFile) and file_size > 0):
          print "file size = " + str(file_size)
          print "chunk size = " + str(chunk_bytes)
          if (file_size <= chunk_bytes):
# please note: If the gridftp server is not started yet, the following operation will fail
            copyFile = "globus-url-copy file://" + str(sendFile) + " gsiftp://" + gridFtpIP
            print copyFile
            os.system(copyFile)
          elif (file_size > chunk_bytes):
            split_command = "split -d -a 3 -b " + str(int(chunk_bytes)) + " " + sendFile + " " + sendFile
            os.system(split_command)
            nr_files = file_size/chunk_bytes + 1
            i=0
# include leading zeros tot he file names accordingly
            while i < nr_files:
              if nr_files <= 9:
                filename2 = sendFile + "00" + str(i)
              elif (nr_files >9 and nr_files <= 99):
                filename2 = sendFile + "0" + str(i)
              elif (nr_files >99 and nr_files <= 999):
                filename2 = sendFile + str(i)

            print filename2
# please note: If the gridftp server is not started yet, the following operation will fail
            copyFile = "globus-url-copy file://" + str(filename2) + " gsiftp://" + gridFtpIP
            print copyFile
            os.system(copyFile)
            i +=1
            continue
        elif (exists(sendFile) and file_size == 0):
          break
        else:
# If file doesn't already exist than download it from Mark5
          mark5_chunk_output = Mark5_get_chunks(host, 
          portMark5Data,
          portMark5Control, 
          ipMark, 
          sendFile, 
          block_size, 
          chunk_real_size, 
          start_position)
          print mark5_chunk_output

        time.sleep(1)

# After the file is splitted or downloaded than notify the grid broker
# The host name below belongs to the host of the Notification service
        print "send notification to grid broker..."
        node_notification = TranslationNodeNotification(brokerIP,
                                                        10001,
                                                        gridFtpIP,
                                                        chunk_real_size,
                                                        chunk_end,
                                                        chunk_start,
                                                        "http://huygens",
                                                        20001)

        print node_notification
        print "end of notification to grid broker..."

# Check if the required chunks are delivered completly
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
# The following statement might be usefull
# Mark5_disconnect()
    return rsp

# The following lines actually starts the service
# the service name is defined here as Service('servicename')
# This service is run at: http://url-of-the-machine:portnumber/servicename
if __name__ == "__main__" :
  port = portNumber
  AsServer(port, (Service('translationnode'),))
