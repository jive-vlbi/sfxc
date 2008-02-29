#!/usr/bin/env python2.4
from ZSI.ServiceContainer import AsServer
from ZSI import ServiceProxy
from TranslationNodeNotification_services_server import *

class Service(TranslationNodeNotification):
  def soap_chunkIsReady(self, ps):
    rsp = TranslationNodeNotification.soap_chunkIsReady(self, ps)    
    msg = self.request

    print "hello Mark..."
    print 'chunk ID: ', msg.Param0._chunkId
    print 'chunk Location: ', msg.Param0._chunkLocation
#    print 'Requested chunk size: ', msg.Param0.chunkSize
#    print 'Requested end time: ', msg.Param0.endTime
#    print 'Requested start time: ', msg.Param0.startTime
#    print 'translationNode IP: ', msg.Param0.translationNodeIP
#    print 'translationNode Id: ', msg.Param0.translationNodeId
        
    return rsp

if __name__ == "__main__" :
	port = 8080
	AsServer(port, (Service('test'),))
