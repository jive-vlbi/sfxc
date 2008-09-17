import sys
import httplib
from TranslationNodeNotificationZSI.TranslationNodeNotification_services import *
from wsutils import camelCase


def makeTranslationNodeNotification(chunkId, chunkLocation, chunkSize, 
                                    endTime, startTime, translationNodeIP, translationNodeId):
    req = chunkIsReadyRequest()
    req.Param0 = req.new_param0()  
    for p in ['chunkId', 'chunkLocation', 
              'chunkSize', 'endTime', 'startTime', 
              'translationNodeIP', 'translationNodeId']:
        setattr(req.Param0, camelCase(p), locals()[p])
    ## We can access these parameters here as follows
    ##     print 'chunk ID: ', req.Param0.ChunkId
    return req
        
def sendTranslationNodeNotification(req, tnn_service, debug=False):
    loc = TranslationNodeNotificationLocator()
    if debug: 
        tracefile = sys.stderr
    else:
        tracefile = None
    port = loc.getTranslationNodeNotificationPortType(tnn_service)
    print >>sys.stdout, "Attempting to contact notification service at", tnn_service
    try:
        port.chunkIsReady(req)
    except httplib.ResponseNotReady:
        print >>sys.stdout, "Axis server returned 202; proceeding"

if __name__=='__main__':
    import simplejson as json, time
    conf = json.load(file("service_conf.js"))
    loc = TranslationNodeNotificationLocator()
    req = makeTranslationNodeNotification(10001, conf['gridFtpBaseURL'], 100000, 
                                          1.0, 2.0, "http://huygens.nfra.nl", 20001)
    tnn_service = conf['notificationService']
    sendTranslationNodeNotification(req, tnn_service)
