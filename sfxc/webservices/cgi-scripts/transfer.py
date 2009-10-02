import os, sys
import re
import httplib, traceback

outputPath = "/sfxc/output"

class GridFTPLocation(object):
    def __init__(self, base, exptName):
        self.base = base
        self.exptName = exptName
        self.rdirpath = os.path.join(self.base, self.exptName.lower())
    def remotePath(self, sendFile):
        basefn = os.path.basename(sendFile)
        gridURL = os.path.join(self.rdirpath, basefn)
        return gridURL
    def transferCommand(self, sendFile):
        gridURL = self.remotePath(sendFile)
        gftpCommand = ('/huygens_1/jops/globus/bin/globus-url-copy -p 4 -cd file://%s %s' %
                       (sendFile, gridURL))
        return gftpCommand
    def transferBackCommand(self):
        # Oh dear.  self.base is a directory if we're sending stuff up to server
        # but a file if we're pulling it down.
        localFN = os.path.join(outputPath, os.path.basename(self.base))
        command = ('/huygens_1/jops/globus/bin/globus-url-copy -p 4 -cd %s file://%s' %
                       (self.base, localFN))
        return command
        

            
class SCPLocation(object):
    def __init__(self, url, exptname):
        """scp://user@host.example.com/dir/path/file.txt"""
        m = re.match("scp://([^/@]*@[^/@]*):(/.*)", url)
        self.url = url
        self.userAtHost, self.base = m.groups()
        self.exptname = exptname
        self.rdirpath = os.path.join(self.base, self.exptname.lower())
    def remotePath(self, sendFile):
        basefn = os.path.basename(sendFile)
        rp = os.path.join(self.rdirpath, basefn)
        return rp
    def transferCommand(self, sendFile):
        rpath = self.remotePath(sendFile)
        return ("ssh %s 'mkdir -p %s'; " 
                "scp %s %s:%s" % 
                (self.userAtHost, self.rdirpath, sendFile, self.userAtHost, rpath))
    def transferBackCommand(self):
        basedir = os.path.join(outputPath, self.exptname.lower())
        localFN = os.path.join(basedir, os.path.basename(self.url))
        return ("mkdir -p %s; scp %s:%s %s" % (basedir, self.userAtHost, self.base, basedir))



def makeRemoteLocation(base, exptname):
    if base.startswith("gsiftp://"):
        return GridFTPLocation(base, exptname)
    elif base.startswith("scp://"):
        return SCPLocation(base, exptname)
    else:
        raise RuntimeError, "No handler for url %s" % base


def sendNotification(port, req):
    try:
        port.update(req)
    except httplib.ResponseNotReady:
        print >>sys.stderr, "Axis server returned 202; continuing"
    except Exception, e:
        print >>sys.stderr, "Port dict:", dir(port)
        print >>sys.stderr, "Failed to send notification to Axis:"
        traceback.print_exc(file=sys.stderr)
