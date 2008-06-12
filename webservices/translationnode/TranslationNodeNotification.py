import sys
from TranslationNodeNotification_services import *
from wsutils import camelCase

class TranslationNodeNotification(object):
    def __init__(self, chunkId, chunkLocation, chunkSize, 
                 endTime, startTime, translationNodeIP, translationNodeId):
        self.req = chunkIsReadyRequest()
        self.req.Param0 = self.req.new_param0()  
        for p in ['chunkId', 'chunkLocation', 
                  'chunkSize', 'endTime', 'startTime', 
                  'translationNodeIP', 'translationNodeId']:
            setattr(self.req.Param0, camelCase(p), locals()[p])
    # We can access these parameters here as follows
    #     print 'chunk ID: ', req.Param0.ChunkId
    #     print 'chunk Location: ', req.Param0.ChunkLocation
    #     print 'Requested chunk size: ', req.Param0.ChunkSize
    #     print 'Requested end time: ', req.Param0.EndTime
    #     print 'Requested start time: ', req.Param0.StartTime
    #     print 'translationNode IP: ', req.Param0.TranslationNodeIP
    #     print 'translationNode Id: ', req.Param0.TranslationNodeId
        
        

if __name__=='__main__':
    import simplejson as json
    conf = json.load(file("service_conf.js"))
    tnn = TranslationNodeNotification(10001, conf['gridFtpIp'], 100000, 
                                      1.0, 2.0, "http://huygens", 20001)
    
    brokerIPAddress = ('http://melisa.man.poznan.pl:8086/'
                       'vlbiBroker/services/TranslationNodeNotification')
    loc = TranslationNodeNotificationLocator()
    port = TranslationNodeNotificationSOAP11BindingSOAP(brokerIPAddress, tracefile=sys.stdout)
    resp = port.chunkIsReady(tnn.req)
    print resp
