import sys
import httplib
#from TranslationNodeNotificationZSI.TranslationNodeNotification_services import *
# 2009-02-09:
# Updated WSDL file and corresponding ZSI stuff:
from TranslationNodeNotificationZSI.TranslationNotification_services import *
from wsutils import camelCase

def makeTranslationNotification(chunkId, chunkLocation, chunkSize, 
                                endTime, startTime, translationNodeIP, translationNodeId):
    req = chunkIsReadyRequest()
    req.ChunkInfo = req.new_chunkInfo()  
    for p in ['chunkId', 'chunkLocation', 
              'chunkSize', 'endTime', 'startTime', 
              'translationNodeIP', 'translationNodeId']:
        setattr(req.ChunkInfo, camelCase(p), locals()[p])
    ## We can access these parameters here as follows
    ##     print 'chunk ID: ', req.Param0.ChunkId
    return req
if __name__=='__main__':
    import simplejson as json, time
    conf = json.load(file("service_conf.js"))
    loc = TranslationNodeNotificationLocator()
    req = makeTranslationNodeNotification(10001, conf['gridFtpBaseURL'], 100000, 
                                          1.0, 2.0, "http://huygens.nfra.nl", 20001)
    tnn_service = conf['notificationService']
