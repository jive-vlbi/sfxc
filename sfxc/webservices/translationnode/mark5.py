import struct, os
SEEK_BEG = 0
SEEK_CUR = 1
SEEK_END = 2

import datetime, time, math

def strptime(s, f):
    return datetime.datetime(*(time.strptime(s, f)[0:6]))

def getsize(f):
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

class Mark5Handler(object):
    def __init__(self, f, timeoffset=0, dataoffset=0):
        self.f = f
        self.fsize = getsize(f)
        self.state = 'unknown'
        self.timeoffset = timeoffset
        self.dataoffset = dataoffset
        self.wordsize = self.getWordSize()
        reftime, refpos, self.tinterval = self.getReferenceTime()
        self.datarate = int(1000000* # microseconds -> seconds
                            20000.0*self.wordsize/ # framesize
                            (toLong(self.tinterval)))
        self.reftime = reftime 
        self.refpos = refpos # - dataoffset?
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
    def get_chunks(self, fileName, blockSize, chunkSize, startPosition):
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

if __name__=='__main__':
    f = open("/data4/sfxc/scans/n08c1/m5a/n08c1_wb_no0017")
    mk5 = Mark5Handler(f)

    times = []
    poses = []
    for i in range(20):
        poses.append(f.tell())
        dt = mk5.extractDatetime()
        times.append(dt)
        try:
            mk5.skipData()
        except EOFError:
            break
    for t, p in zip(times, poses):
        t1 = mk5.bytes_starting_position(p)
        print t, p, t1
    size = 40*1024
    start = 1024
    fn = "chunk1"
    mk5.get_chunks("chunk1", 4096, size, start)
    f.seek(start)
    s1 = f.read(size)
    f2 = open(fn)
    s2 = f2.read(size)
    if s1!=s2:
        raise RuntimeError, "chunking fails"
