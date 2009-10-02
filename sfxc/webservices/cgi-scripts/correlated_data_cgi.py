#!/usr/bin/env python2.4
import cgi, cgitb
cgitb.enable()
import sys, re, time, os, math, glob
import traceback, datetime, threading
import Numeric as Nu, itertools
import subprocess, time
sys.path = ([sys.path[0]] + 
            ['/home/small/lib64/python2.4/site-packages', 
             '/home/small/code/share/lib/python2.4/site-packages',
             '/home/small/code/share/lib/python2.4/site-packages/'
             'antlr_python_runtime-3.0.1-py2.4.egg',
             '/home/kruithof/share/lib64/python2.4/site-packages',
             '/home/kruithof/share/lib64/python2.4/site-packages/'
             'simplejson-1.7.3-py2.4-linux-x86_64.egg',
             '/home/small/code/webservices/vex2ccf_web_service',
             '/home/small/code/webservices/translationnode']
            + sys.path[1:])
sys.path.insert(1, '/home/small/webservices')

from Notification.Notification import *

import simplejson as json
sys.path.insert(1, '/home/small/code/webservices/translationnode')
import mark5 as mk5tools
import TranslationNode_vex as vextools
from transfer import makeRemoteLocation, sendNotification

conf =  json.load(file('/home/small/code/webservices/translationnode/service_conf.js'))
portMark5Data = conf['portMark5Data']
portMark5Control = conf['portMark5Control']
ipMark = conf['ipMark']
host = conf['host']
outputPath = conf['outputPath'] # for correlated data

def dataSetIsReady(p):
    """{'dataSetInfo': {'header': 
    {'senderLocation': u'Sender location not used.',
    'callbackLocation': u'http://192.42.120.69:8080/vlbiBroker/services/Notification', 
    'experimentName': u'n08c1', 
    'jobId': u'001'}, 
    'chunkInfo': 
    {'chunkId': u'1', 
    'chunkLocation': u'gsiftp://expres.reef.man.poznan.pl/opt/expres/data/N08C1-001/N08C1-001-1.cor', 
    'chunkCount': u'14'}}}"""

    
    dsi = p['dataSetInfo']
    header = dsi['header']
    header["senderCode"] = "cds"
    header["senderLocation"] = "http://192.42.120.69:8081/services.py"
    callBack = header['callbackLocation']
    exptName = header['experimentName'].upper()
    chunkInfo = dsi['chunkInfo']
    # We get everything as strings, but some things
    # should be int.  We ignore the wsdl, so we don't know that.
    chunkInfo['chunkId'] = int(chunkInfo['chunkId'])
    chunkInfo['chunkSize'] = int(chunkInfo['chunkSize'])

    chunkInfo['chunkCount'] = int(chunkInfo['chunkCount'])
    rlocation = chunkInfo['chunkLocation']
    lpath = os.path.join(outputPath, exptName, os.path.basename(rlocation))

    notification_loc = NotificationLocator()
    port = notification_loc.getNotificationPortType(callBack)
    print >>sys.stderr, datetime.datetime.now(), "Welcome to CorrelatedDataNotification (CGI Edition)"
    print >>sys.stderr, p
    try: 
        remoteLocation = makeRemoteLocation(rlocation, exptName)
        print >>sys.stderr, "remoteLocation", remoteLocation
    except RuntimeError:
        state = {"description": "No handler for %s." % rlocation,
                 "nameKey" : "state.error"}
        req = makeNotification({"chunkInfo" : chunkInfo,
                                "header" : header,
                                "message" : "Pointless message just to test",
                                "state" : state,
                                })
        sendNotification(port, req)

    transferCommand = remoteLocation.transferBackCommand()
    print >>sys.stderr, "Command:", transferCommand
    retcode = subprocess.call(transferCommand, shell=True)
    print >>sys.stderr, "Retcode:", retcode
    if retcode != 0:
        print >>sys.stderr, "Transfer failed"
        state = {"name" : "Error",
                 "nameKey" : "state.error"}
    else:
        print >>sys.stderr, "Transfer succeded"
        state = {"name" : "OK",
                 "nameKey" : "state.done"}
    req = makeNotification({"chunkInfo" : chunkInfo,
                            "header" : header,
                            "message" : "Pointless message just to test",
                            "state" : state,
                            })
    sendNotification(port, req)
    print >>sys.stderr, "Notification sent."

def forkDataSetIsReady(p):
    try:                                  
        pid = os.fork()
        if pid==0: # child
            os.close(sys.stderr.fileno())
            os.close(sys.stdout.fileno())
            sys.stderr = open('/home/small/log/tnn/err.log', 'aw', 0)
            print >>sys.stderr, "New error stream"
            dataSetIsReady(p)
    except Exception, e:
        print >>sys.stderr, "Failed to send notification to Axis:"
        traceback.print_exc(file=sys.stderr)


if __name__=="__main__":
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
        CorrelatedDataNotification(p)
