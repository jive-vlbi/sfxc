import struct, os, time, datetime
import simplejson, glob
import TranslationNode_vex as vextools

SEEK_BEG = 0
SEEK_CUR = 1
SEEK_END = 2

import datetime, time, math

conf =  simplejson.load(file('/home/small/code/webservices/translationnode/service_conf.js'))

def strptime(s, f):
    return datetime.datetime(*(time.strptime(s, f)[0:6]))

def getfilesize(f):
    pos = f.tell()
    f.seek(0, SEEK_END)
    end = f.tell()
    f.seek(pos, SEEK_BEG)
    return end


millisecondTable = {'0' : 0,
                    '1' : 1.25,
                    '2' : 2.5,
                    '3' : 3.75,
                    '5' : 5,
                    '6' : 6.25,
                    '7' : 7.5,
                    '8' : 8.75, }


def toLong(dt):
    return (1000000*(dt.days*24*60*60+
                     dt.seconds*1000) +
            dt.microseconds)

def fromLong(l):
    days, l = divmod(l, 24*60*60*1000000)
    secs, ms = divmod(l, 1000000)
    return datetime.timedelta(days, secs, ms)

def formatTime(t):
    return t.strftime("%Yy%jd%Hh%Mm%S.")+("%04d" % (t.microsecond//100))+'s'

# /data4/sfxc/scans/n07c1/m5a + ef =>

# n07c1_ef_no0101.m5a  
# n07c1_ef_no0117.m5a  
# n07c1_ef_no0119.m5a  

blockSize = conf["block_size"]

class Mark5Emulator(object):
    def __init__(self, experiment_name, station, vex):
        mark5scansPath = conf["mark5scansPath"]
        self.vex = vex
        self.fns = glob.glob(os.path.join(mark5scansPath, 
                                          "%s/m5a/%s_%s_*" % (experiment_name.lower(), 
                                                              experiment_name.lower(),
                                                              station.lower())))
        if not self.fns:
            s = ("No files for experiment %s, station %s in %s" % 
                 (experiment_name, station, mark5scansPath))
            raise RuntimeError, s
        self.handlers = [Mark5ScanHandler(fn) for fn in self.fns]
    def getScanStart(self, scan):
        return self.vex['SCHED'][scan]['start']
    def getChunksByTime(self, oFilename, chunk_start, chunk_end):
        dtdt = datetime.datetime.fromtimestamp
        for h in self.handlers:
            print "Get chunks by time", dtdt(chunk_start), dtdt(chunk_end), h.reftime, h.tinterval
            if (h.reftime < dtdt(chunk_start) and dtdt(chunk_end) < h.endtime):
                break
        else:
            raise RuntimeError, "Chunk not inside a scan: %s, %s, %s" % (str(self.handlers),
                                                                        dtdt(chunk_start),
                                                                        dtdt(chunk_end))
        return h.getChunksByTime(oFilename, chunk_start, chunk_end)
    def disconnect(self):
        pass

class Mark5ScanHandler(object):
    def __init__(self, fn):
        self.fn = fn
        self.f = file(fn)
        self.fsize = getfilesize(self.f)
        self.state = 'unknown'
        self.wordsize = self.getWordSize()
        self.reftime, self.refpos, self.tinterval = self.getReferenceTime()
        self.datarate = int(1000000* # microseconds -> seconds
                            20000.0*self.wordsize/ # framesize
                            (toLong(self.tinterval)))
        self.endtime = self.reftime + datetime.timedelta(seconds=self.fsize/self.datarate)
    def timeToByte(self, t):
        return self.refpos+(t-time.mktime(self.reftime.timetuple()))*self.datarate
    def getWordSize(self):
        count = 0
        while True:  
            s = struct.unpack("B", self.f.read(1)) 
            if s == (int('ff', 16),): 
                count += 1
            else:
                if count > 31:
                    print "Count:", count
                    self.f.seek(-1, SEEK_CUR)
                    break
                count = 0
        wordsize, rem = divmod(count, 32)
        if rem != 0:
            print "Correcting byte alignment"
            self.f.seek(-wordsize, SEEK_CUR)
        self.f.seek(-wordsize*(64+32), SEEK_CUR)
        self.state = 'startOfHeader'
        return wordsize
    def rewindHeader(self):
        if self.state != 'startOfData':
            raise RuntimeError, "Not at start of data"
        self.f.seek(-(64+32+64)*self.wordsize, SEEK_CUR)
        self.state = 'startOfHeader'
    def getReferenceTime(self):
        if self.state != 'startOfHeader':
            raise RuntimeError, "Not at start of header"
        t0 = self.extractDatetime()
        self.rewindHeader()
        self.skipFrame()
        t1 = self.extractDatetime()
        self.rewindHeader()
        self.skipFrame(-1)
        pos = self.f.tell()
        return t0, pos, t1-t0
    def extractDatetime(self):
        if self.state != 'startOfHeader':
            raise RuntimeError, "Not at start of header"
        self.f.seek(self.wordsize*(64+32), SEEK_CUR)
        header = struct.unpack("%dB" % 64*self.wordsize, 
                               self.f.read(64*self.wordsize))
        bits = [(header[i] & 1) for i in range(0, len(header), self.wordsize)]
        l = []
        for i in range(0, 13*4, 4):
            d = int("".join(str(b) for b in bits[i:i+4]), 2)
            l.append(d)
        ## FIXME: it is not always 200d for single digit d
        datestring = ''.join([str(d) for d in l])
        dt = (strptime('200'+datestring[:10], "%Y%j%H%M%S")+
              datetime.timedelta(milliseconds=10*int(datestring[10:12])+
                                 millisecondTable[datestring[12]]))
        self.state = 'startOfData'
        return dt
    def skipData(self):
        if self.state != 'startOfData':
            raise RuntimeError, "Not at start of data"
        offset = (20000-64-32-64)*self.wordsize
        if self.f.tell()+offset > self.fsize:
            raise EOFError, "Incomplete data segment"
        self.f.seek(offset, SEEK_CUR)
        self.state = 'startOfHeader'
    def skipFrame(self, nframes=1):
        if self.state != 'startOfHeader':
            raise RuntimeError, "Not at start of header"
        offset = nframes*20000*self.wordsize
        self.f.seek(offset, SEEK_CUR)
        self.state = 'startOfHeader'
    ## Husseyin's interface below
    def getChunksByTime(self, filename, chunk_start, chunk_end):
        startPosition = self.timeToByte(chunk_start)
        chunk_real_size = int(self.datarate*(chunk_end-chunk_start))
        self.get_chunks(filename, chunk_real_size, startPosition)
        return chunk_real_size
    def get_chunks(self, fileName, chunkSize, startPosition):
        of = open(fileName, "w")
        self.f.seek(startPosition)
        pos = startPosition
        while True:
            size = min(blockSize, startPosition+chunkSize-pos)
            size = max(size, 0)
            data = self.f.read(size) 
            if len(data)==0:
                break
            of.write(data)
            pos += len(data)

if __name__=="__main__":
    m = Mark5Emulator("n08c1", "wb", "")
    h = m.handlers[0]
    print h.reftime, h.tinterval
