#!/usr/bin/env python2.4
import cgi, cgitb
cgitb.enable()
import sys, re, time, os, math, glob
import traceback, datetime, threading
import Numeric as Nu, itertools
import subprocess, time
import operator, re

sys.path = ([sys.path[0]] + 
            ['/home/small/lib64/python2.4/site-packages', # ZSI
             '/home/small/code/webservices/translationnode']
            + sys.path[1:])

sys.path.insert(1, '/home/small/webservices')
from transfer import makeRemoteLocation, sendNotification
from Notification.Notification import *

import simplejson as json
sys.path.insert(1, '/home/small/code/webservices/translationnode')
import mark5 as mk5tools
import TranslationNode_vex as vextools

conf =  json.load(file('/home/small/code/webservices/translationnode/service_conf.js'))
portMark5Data = conf['portMark5Data']
portMark5Control = conf['portMark5Control']
ipMark = conf['ipMark']
host = conf['host']
localPath = conf['localPath']
portNumber = conf['portNumber']
vexFilePath = conf["vexFilePath"]

def pairwise(l):
    for p in itertools.izip(l[:-1], l[1:]):
        yield p

def fixStartTime(chunk_times, scan_data_start):
    for i in range(len(chunk_times)-1):
        if chunk_times[i] < scan_data_start < chunk_times[i+1]:
            break
    new_times = [scan_data_start] + chunk_times[i+1:]
    return new_times            

class TimeToByteConverter(object):
    def __init__(self, time, byte, datarate):
        quant_time, frac_time = math.modf(time)
        quant_byte = byte - frac_time*datarate
        self.time = quant_time
        self.byte = quant_byte
        self.datarate = datarate
    def timeToByte(self, time):
        return int(self.byte + (time - self.time)*self.datarate)

def vexGetDataRate(vex, scan, station):
    # Calculate the data rate for this scan.
    mode = vex.get_mode(scan)
    bits_per_sample = vex.get_bits_per_sample(mode, station)
    num_channels = vex.get_num_channels(mode, station)
    sample_rate = vex.get_sample_rate(mode, station)
    data_format = vex.get_data_format(scan, station)
    overhead = vextools.data_format_overhead[data_format]
    data_byte_rate = num_channels * sample_rate * bits_per_sample * overhead / 8
    return data_byte_rate

class Sched(object):
    def __init__(self, sched):
        self.sched = sched
    def getScans(self):
        return list(self.sched.iterkeys())
    def getScanRange(self, scan):
        scan_start = vextools.parse_vex_time(self.sched[scan]['start'])
        # Find out the true length of the scan, which is just the maximum
        # of the per-station length of the scan.
        def extractLength(info):
            return float(info[2].split()[0])
        secs = max([extractLength(info) for info in self.sched[scan].getall('station')])
        scan_end = scan_start + secs
        return scan_start, scan_end
    def isScanRelevant(self, scan, job_start, job_end):
        scan_start, scan_end = self.getScanRange(scan)
        res = not (job_start > scan_end  or scan_start > job_end)
        print >>sys.stderr, "isScanRelevant: ", vextools.format_vex_time(job_start), \
            vextools.format_vex_time(job_end), scan, \
            vextools.format_vex_time(scan_start), vextools.format_vex_time(scan_end), res
        return res


def modal(l):
    d = {}
    for e in l:
        d[e] = d.get(e, 0) + 1
    max = 0
    res = None
    for e, count in d.iteritems():
        if count > max:
            res = e
            max = count
    return res
        
class Chunker(object):
    def __init__(self, station, vex_fn, exptname, 
                 job_start, job_end, requested_chunk_size):
        self.station = station
        self.vex_fn = vex_fn
        self.exptname = exptname
        self.job_start = job_start
        self.job_end = job_end
        print >>sys.stderr, "Job start and end", vextools.format_vex_time(job_start), \
            vextools.format_vex_time(job_end)
        self.requested_chunk_size = requested_chunk_size
        self.vex = vextools.Vex(self.vex_fn)
        self.sched = Sched(self.vex['SCHED'])
        self.relevant_scans = [scan for scan in self.sched.getScans() 
                               if self.sched.isScanRelevant(scan, self.job_start, self.job_end)]

        self.modal_byte_rate = modal([vexGetDataRate(self.vex, scan, self.station) 
                                      for scan in self.relevant_scans])
        self.modal_chunk_length = self.quantizeChunkDT(self.requested_chunk_size,
                                                  self.modal_byte_rate)
        self.n_chunks_est = int((self.job_end - self.job_start) 
                                // self.modal_chunk_length)

    def quantizeChunkDT(self, requested_chunk_size, data_byte_rate):
        # chunk_length should be an *integer*(>0) number of seconds
        chunk_length = max(math.floor(requested_chunk_size/data_byte_rate), 1.0)
        return chunk_length
    def __iter__(self):
        ## Real mark5:
        ## mark5 = mk5tools.Mark5(portMark5Control, ipMark, experiment_name, station)
        ## Fake mark5
        mark5 = mk5tools.Mark5Emulator(self.exptname, self.station, self.vex)
        globalChunkNumber = 0
        for scan in self.relevant_scans:
            print >>sys.stderr, "Scan:", scan
            scan_start, scan_end = self.sched.getScanRange(scan)
            data_byte_rate = vexGetDataRate(self.vex, scan, self.station)
            scan_size_bytes = (scan_end-scan_start)*data_byte_rate
            if (self.requested_chunk_size == 0):
                chunk_dt = self.quantizeChunkDT(scan_size_bytes, data_byte_rate)
            else:
                chunk_dt = self.quantizeChunkDT(self.requested_chunk_size, data_byte_rate)
            print >>sys.stderr, "Scan boundaries:", 
            print >>sys.stderr, vextools.format_vex_time(scan_start), 
            print >>sys.stderr, vextools.format_vex_time(scan_end)
            print >>sys.stderr, "Job boundaries:", 
            print >>sys.stderr, vextools.format_vex_time(self.job_start), 
            print >>sys.stderr, vextools.format_vex_time(self.job_end)
            scan_data_start = vextools.parseFractionalTime(mark5.getScanStart(scan))
            start_time = max(self.job_start, scan_start)
            end_time = min(self.job_end, scan_end)

            if scan_data_start > end_time:
                print >>sys.stderr,"scan_data_start > end_time: Skipping", 
                print >>sys.stderr, (vextools.format_vex_time(scan_data_start), 
                                     vextools.format_vex_time(end_time))
                continue
            print >>sys.stderr, "t1, t2, dt", 
            print >>sys.stderr, vextools.format_vex_time(start_time), \
                                 vextools.format_vex_time(end_time), chunk_dt
            chunk_times = Nu.arange(start_time, end_time, chunk_dt).tolist() + [end_time]
            if scan_data_start > start_time:
                chunk_times = fixStartTime(chunk_times, scan_data_start)
            print >>sys.stderr, "Chunk times:", ", ".join([vextools.format_vex_time(t) 
                                                           for t in chunk_times])
            for i, (chunk_start, chunk_end) in enumerate(pairwise(chunk_times)):
                sendFile = os.path.join(localPath, (self.exptname + '_' + self.station + '_' + 
                                                    str(scan) + "_" + "%03d" % i + '.m5a').lower())
                print >> sys.stderr, "Filename:", sendFile
                chunk_real_size = mark5.getChunksByTime(sendFile, chunk_start, chunk_end)
                yield (sendFile, globalChunkNumber, chunk_real_size, 
                       vextools.format_vex_time(chunk_start), 
                       vextools.format_vex_time(chunk_end))
                globalChunkNumber += 1

        print >>sys.stderr, "Disconnecting from mark5"
        mark5.disconnect()



def startTranslationJob(p):
    """{'translationJobInfo': {'dataLocation': 
                            u'gsiftp://expres.reef.man.poznan.pl/opt/expres/data/', 
                            'startTime': u'2008y070d17h10m00s', 
                            'endTime': u'2008y070d17h10m10s', 
                            'chunkInfo': {'chunkSize': u'1024', 
                                          'chunkId': u'1', 
                                          'telescopeAbbr': u'Ef', 
                                          'chunkLocation': u'gsiftp://expres.reef.man.poznan.pl/opt/expres/data/',
                                          'chunkStartTime': u'2008y070d17h10m00s',
                                          'chunkEndTime': u'2008y070d17h10m10s', 
                                          'chunkCount': u'14'}, 
                            'header': {'senderLocation': u'Sender location not used.', 
                                       'callbackLocation': u'http://10.88.0.190:8080/vlbiBroker/services/Notification',
                                       'experimentName': u'N08C1', 
                                       'jobId': u'JOB_ID_99288'}}}"""

    print >>sys.stderr, datetime.datetime.now(), "Welcome to startTranslationNode (CGI Edition)"
    print >>sys.stderr, p
    try:
        param = p["translationJobInfo"]
        job_start = vextools.parse_vex_time(param["startTime"])
        job_end = vextools.parse_vex_time(param["endTime"])

        header = param["header"]
        header["senderLocation"] = "http://192.42.120.69:8081/services.py"
        header["senderCode"] = "tn"
        callBack = header["callbackLocation"]
        experiment_name = header["experimentName"]
        job_id = header["jobId"]

        chunk_info = param["chunkInfo"]
        station = chunk_info["telescopeAbbr"]
        requested_chunk_size = int(chunk_info["chunkSize"])
        tnn_loc = NotificationLocator()
        port = tnn_loc.getNotificationPortType(callBack)

        try:
            remoteLocation = makeRemoteLocation(param["dataLocation"], experiment_name)
        except RuntimeError:
            state = {"description": "No handler for %s." % param["dataLocation"],
                     "nameKey" : "state.error"}
            telescope = chunk_info["telescopeAbbr"]
            chunkInfo = {'telescopeAbbr': telescope}
            req = makeNotification({"state" : state,
                                    "header" : header, 
                                    "chunkInfo" : chunkInfo})
            sendNotification(port, req)
            sys.exit(1)
            
        vex_file_name = os.path.join(vexFilePath, experiment_name.lower() + ".vix")

        print >>sys.stderr, "Port dict:", dir(port)
        print >>sys.stderr, "Decoded request"
        print >>sys.stderr, "Got TranslationNodeNotifier"


        if os.path.exists(vex_file_name) and os.path.isfile(vex_file_name):
            state = {"description": "Translation request successfully decoded.",
                     "nameKey" : "state.ok"}
            req = makeNotification({"state" : state,
                                    "header" : header})
            sendNotification(port, req)
        else:
            state = {"description": "No vexfile for %s." % experiment_name,
                     "nameKey" : "state.error"}
            req = makeNotification({"state" : state,
                                    "header" : header})
            sendNotification(port, req)
            sys.exit(1)

        chunks = Chunker(station, vex_file_name, experiment_name, 
                         job_start, job_end, requested_chunk_size)

        translation_node_ip = "http://huygens.nfra.nl"

        for i, tup in enumerate(iter(chunks)):
            (sendFile, chunk_id, chunk_real_size, chunk_start, chunk_end) = tup
            transferCommand = remoteLocation.transferCommand(sendFile)
            print >>sys.stderr, "Command:", transferCommand
            remotePath = remoteLocation.remotePath(sendFile)
            chunk_info = {"chunkCount" : chunks.n_chunks_est, 
                          "chunkId" : chunk_id,
                          "chunkLocation" : remotePath,
                          "chunkSize" : chunk_real_size,
                          "chunkStartTime" : chunk_start,
                          "chunkEndTime" : chunk_end,
                          "telescopeAbbr" : station,
                          }
            retcode = subprocess.call(transferCommand, shell=True)
            if retcode != 0:
                print >>sys.stderr, "File transfer failed"
                state = {"description" : "File transfer failed",
                         "nameKey" : "state.error"}
                breakOut = True
            else:
                state = {"nameKey" : "state.tn.notification"}
                breakOut = False
            print >>sys.stderr, "notifying grid broker for chunk %d of %d" % (i, 
                                                                              chunks.n_chunks_est)
            # already have header
            req = makeNotification({"chunkInfo" : chunk_info, 
                                    "header" : header,
                                    "state" : state })
            try:
                port.update(req)
            except httplib.ResponseNotReady:
                print >>sys.stdout, "Axis server returned 202; continuing"
            except Exception, e:
                print >>sys.stderr, "Failed to send notification to Axis:"
                traceback.print_exc(file=sys.stderr)

            if breakOut: break

        if i == 0 and not breakOut:
            # We handle the empty case here.
            state = {"description": "No data for this time-range.",
                     "nameKey" : "state.error"}
            req = makeNotification({"state" : state,
                                    "header" : header})
            sendNotification(port, req)
            sys.exit(1)
            

        print >>sys.stderr, "sent all notifications to grid broker..."
        state = {"nameKey" : "state.done"}
        telescope = chunk_info["telescopeAbbr"]
        chunkInfo = {'telescopeAbbr': telescope}
        req = makeNotification({"header" : header,
                                "state" : state,
                                "chunkInfo": chunkInfo})
        try:
            port.update(req)
        except httplib.ResponseNotReady:
            print >>sys.stdout, "Axis server returned 202; continuing"
        except Exception, e:
            print >>sys.stderr, "Failed to send notification to Axis:"
            traceback.print_exc(file=sys.stderr)

    except:
        traceback.print_exc(file=sys.stderr)
        

def forkTranslationJob(p):
    pid = os.fork()
    if pid==0: # child
        os.close(sys.stderr.fileno())
        os.close(sys.stdout.fileno())
        sys.stderr = open('/home/small/log/tnn/err.log', 'aw', 0)
        print >>sys.stderr, "New error stream"
        startTranslationJob(p)


from StringIO import StringIO

if __name__=="__main__":
    try:
        fs = cgi.FieldStorage()
        print >>sys.stderr, datetime.datetime.now(), fs
        param = fs.getfirst("param", {})
        p = json.loads(param)
        print >>sys.stdout, "Content-Type: text/html\n"
        print >>sys.stdout, "Got %s" % p
        pid = os.fork()
        if pid==0: # child
            os.close(sys.stderr.fileno())
            os.close(sys.stdout.fileno())
            sys.stderr = open('/home/small/log/tnn/err.log', 'aw', 0)
            print >>sys.stderr, "New error stream"
            startTranslationJob(p)
    except Exception, e:
        print >>sys.stderr, "Something went horribly wrong"
        traceback.print_exc(file=sys.stderr)
