#!/usr/bin/env python2.4
import sys, re, time, os, math
import Numeric as Nu
import simplejson
from ZSI.ServiceContainer import AsServer
from NewTranslationJob_services_server import *
from TranslationNodeNotification import TranslationNodeNotification
# import TranslationNode_mark5 as mk5tools
import mark5 as mk5tools
import TranslationNode_vex as vextools

conf =  simplejson.load(file('service_conf.js'))
portMark5Data = conf['portMark5Data']
portMark5Control = conf['portMark5Control']
ipMark = conf['ipMark']
host = conf['host']
gridFtpIp = conf['gridFtpIp']
fileName = conf['fileName']
block_size = conf['block_size']
portNumber = conf['portNumber']


class TimeToByteConverter(object):
    def __init__(self, time, byte, datarate):
        quant_time, frac_time = math.modf(time)
        quant_byte = byte - frac_time*datarate
        self.time = quant_time
        self.byte = quant_byte
        self.datarate = datarate
    def timeToByte(self, time):
        return int(self.byte + (time - self.time)*self.datarate)

def getDataRate(vex, scan, station):
    # Calculate the data rate for this scan.
    mode = vex.get_mode(scan)
    bits_per_sample = vex.get_bits_per_sample(mode, station)
    num_channels = vex.get_num_channels(mode, station)
    sample_rate = vex.get_sample_rate(mode, station)
    data_format = vex.get_data_format(scan, station)
    overhead = vextools.data_format_overhead[data_format]
    data_byte_rate = num_channels * sample_rate * bits_per_sample * overhead / 8
    print "bits per sample:", bits_per_sample
    print "number of channels:", num_channels
    print "sample rate:", sample_rate
    print "data rate (bytes):", data_byte_rate 
    return data_byte_rate

def parseStartTime(scan_data_start):
    scan_data_start_ms = re.search("\.(\d{4})s", scan_data_start)
    scan_data_start_ms = float("0." + scan_data_start_ms.group(1))
    scan_data_start = re.sub("\.\d{4}","", scan_data_start)
    scan_data_start = vextools.parse_vex_time(scan_data_start)
    return scan_data_start, scan_data_start_ms

def getScanRange(sched, scan):
    scan_start = vextools.parse_vex_time(sched[scan]['start'])
    # Find out the true length of the scan, which is just the maximum
    # of the per-station length of the scan.
    def extractLength(info):
        return float(info[2].split()[0])
    secs = max([extractLength(info) for info in sched[scan].getall('station')])
    scan_end = scan_start + secs
    return scan_start, scan_end

def ackJob(p):
    print 'Requested broker IP address: ', p.BrokerIPAddress
    print 'Requested start time: ', p.StartTime
    print 'Requested end time: ', p.EndTime
    print 'Requested chunk size: ', p.ChunkSize
    print 'Requested data location: ', p.DataLocation[0]
    print 'Requested telescope name: ', p.TelescopeName
    print 'Requested experiment name: ', p.ExperimentName
    print 'client address: ', p.BrokerIPAddress

def quantizeChunkDT(requested_chunk_size, data_byte_rate):
    # chunk_length should be an *integer*(>0) number of seconds
    chunk_length = max(math.floor(requested_chunk_size/data_byte_rate), 1.0)
    return chunk_length

class Service(NewTranslationJob):
    def soap_startTranslationJob(self, ps):
	rsp = NewTranslationJob.soap_startTranslationJob(self, ps)
        print ps
	param = self.request.Param0
        ackJob(param)

	station = param.TelescopeName
	host = param.BrokerIPAddress
	requested_chunk_size = param.ChunkSize
	vex_file_name = param.DataLocation[0]
	experiment_name = param.ExperimentName
	job_start = vextools.parse_vex_time(param.StartTime)
	job_end = vextools.parse_vex_time(param.EndTime)
	vex = vextools.Vex(vex_file_name)
	sched = vex['SCHED']
        
        #mark5 = mk5tools.Mark5(portMark5Control, ipMark)
        
        mark5 = mk5tools.Mark5Handler(file("/data4/sfxc/scans/%s/m5a/%s_%s_no0017" % (experiment_name.lower(), 
                                                                                     experiment_name.lower(),
                                                                                     station.lower())))
        fileFlag = True
        scanList = list(sched.iterkeys())
	for scan in scanList:
            scan_start, scan_end = getScanRange(sched, scan)
            # Skip scans outside of job interval

            if scan_end < job_start or scan_start > job_end:
                continue
            print "Scan:", scan
            data_byte_rate = getDataRate(vex, scan, station)
            scan_size_bytes = (scan_end-scan_start)*data_byte_rate
            if (requested_chunk_size == 0):
                chunk_dt = quantizeChunkDT(scan_size_bytes, data_byte_rate)
            else:
                chunk_dt = quantizeChunkDT(requested_chunk_size, data_byte_rate)
            print "Chunk interval:", chunk_dt
            print "Scan boundaries:", 
            print vextools.format_vex_time(scan_start), vextools.format_vex_time(scan_end)
            if fileFlag:
                st = vextools.format_vex_time(scan_start)
                scan_data_start = st[:-1]+'.0000s'
                start_position = mark5.timeToByte(scan_start)
            else:
                start_position = 1e9*vex.get_data_start(scan, station)//(64*8) # bits or bytes?
                scan_data_start = mark5.initializeScan(start_position)
            print "Scan data start", scan_data_start
            print "Start position", start_position 
            scan_data_start, scan_data_start_ms = parseStartTime(scan_data_start) 
            print "Time:", scan_data_start + scan_data_start_ms
            conv = TimeToByteConverter(scan_data_start+scan_data_start_ms, start_position, data_byte_rate) 
            if scan_data_start_ms > chunk_dt: # chunk_dt is +ve integer seconds
                raise RuntimeError, "Fractional time %f(s) larger than chunk length %f(s)" %(scan_data_start_ms, chunk_dt)
            # get chunk bytes
            end_time = min(job_end, scan_end)

            if scan_data_start > end_time:
                print "Skipping", 
                print vextools.format_vex_time(scan_data_start), vextools.format_vex_time(end_time)
            print "starts:", 
            print vextools.format_vex_time(job_start), vextools.format_vex_time(scan_data_start)
            print "starts2:", job_start, scan_data_start
            if job_start > scan_data_start:  ## HERE
                print "Chunking 1:", 
                print vextools.format_vex_time(job_start), vextools.format_vex_time(end_time), chunk_dt
                chunk_times = Nu.arange(job_start, end_time, chunk_dt).tolist()
            else:
                print "Chunking 2:", 
                print vextools.format_vex_time(scan_data_start), vextools.format_vex_time(end_time), chunk_dt
                chunk_times = Nu.arange(scan_data_start, end_time, chunk_dt).tolist()
                chunk_times[0] = scan_data_start + scan_data_start_ms
            chunk_times.append(end_time)
            print "Chunk times:", ", ".join([vextools.format_vex_time(t) for t in chunk_times])
            for i, (ct1, ct2) in enumerate(zip(chunk_times[:-1], chunk_times[1:])):
                fn = os.path.join(fileName, (experiment_name + '_' + station + '_' + 
                                             str(scan) + "_" + "%03d" % i + '.m5a').lower())
                sendFile = fn
                b1 = conv.timeToByte(ct1)
                chunk_real_size = int(data_byte_rate*(ct2-ct1))
                print sendFile, block_size, chunk_real_size, b1, ct1
                mark5.get_chunks(sendFile, block_size, chunk_real_size, b1)
                time.sleep(1)
            print "send notification to grid broker..."
            node_notification = TranslationNodeNotification(host,
                                                            10001,
                                                            gridFtpIP,
                                                            chunk_real_size,
                                                            chunk_end,
                                                            chunk_start,
                                                            "http://huygens",
                                                            20001)

            print node_notification
            print "end of notification to grid broker..."

        
##           Mark5_disconnect()
        return rsp

if __name__ == "__main__" :
    AsServer(portNumber, (Service('translationnode'),))
