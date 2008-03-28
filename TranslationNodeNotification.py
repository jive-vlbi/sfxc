import sys
from TranslationNodeNotification_services import *

class TranslationNodeNotification:
  def __init__(self):
    self.resp_list=[]

  def __init__(self, BrokerIPAdress=None, 
               chunkId = None, 
               chunkLocation = None, 
               chunkSize = None, 
               endTime = None, 
               startTime = None, 
               translationNodeIP = None, 
               translationNodeId = None):
    
    """ This is a TranslationNodeNotification service simulator. 
    This service is used to test if the TranslationNodeService is sending
    the notification correctly after the downloading/copying from Mark5 
    operations are finished. """
    
  # define request
    req = chunkIsReadyRequest()
    req.Param0 = req.new_param0()  

  # get broker ip address from the read file
#    req.Param0.BrokerIPAddress = BrokerIPAddress
    req.Param0.ChunkId = chunkId
    req.Param0.ChunkLocation = chunkLocation
    req.Param0.ChunkSize = chunkSize
    req.Param0.EndTime = endTime
    req.Param0.StartTime = startTime
    req.Param0.TranslationNodeIP = translationNodeIP
    req.Param0.TranslationNodeId = translationNodeId

# We can access these parameters here as follows
    print 'chunk ID: ', req.Param0.ChunkId
    print 'chunk Location: ', req.Param0.ChunkLocation
    print 'Requested chunk size: ', req.Param0.ChunkSize
    print 'Requested end time: ', req.Param0.EndTime
    print 'Requested start time: ', req.Param0.StartTime
    print 'translationNode IP: ', req.Param0.TranslationNodeIP
    print 'translationNode Id: ', req.Param0.TranslationNodeId

 # get port number from the read file
 # not sure if the port number is necessary
    portNumber_string = "8086"
    portNumber = portNumber_string.strip()
    print portNumber

# get a port proxy instance
# test if the http address correctly parsed
    loc = TranslationNodeNotificationLocator()
 #portTest = 'http://jop32:' + portNumber + '/notification'
    serviceLocation = BrokerIPAdress
    port = loc.getTranslationNodeNotificationPortType(serviceLocation, tracefile=sys.stdout)
    print serviceLocation

 # actualy ask the service to do the job
    resp = port.chunkIsReady(req)
