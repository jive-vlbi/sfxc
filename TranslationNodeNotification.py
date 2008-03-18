from TranslationNodeNotification_services import *

class TranslationNodeNotification:
  def __init__(self, BrokerIPAdress=None, 
               chunkId = None, 
               chunLocation = None, 
               chunSize = None, 
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
    req.Param0.BrokerIPAddress = BrokerIPAddress
    req.Param0.ChunkId = chunkId
    req.Param0.ChunkLocation = chunkLocation
    req.Param0.ChunkSize = chunkSize
    req.Param0.EndTime = endTime
    req.Param0.StartTime = startTime
    req.Param0.TranslationNodeIP = translationNodeIP
    req.Param0.TranslationNodeId = translationNodeId


    print 'chunk ID: ', req.Param0.ChunkId
    print 'chunk Location: ', req.Param0.ChunkLocation
    print 'Requested chunk size: ', req.Param0.ChunkSize
    print 'Requested end time: ', req.Param0.EndTime
    print 'Requested start time: ', req.Param0.StartTime
    print 'translationNode IP: ', req.Param0.TranslationNodeIP
    print 'translationNode Id: ', req.Param0.TranslationNodeId

  # get port number from the read file
    portNumber_string = "8080"
    portNumber = portNumber_string.strip()
    print portNumber

  # test if the http address correctly parsed
    loc = TranslationNodeNotificationLocator()
    portTest = 'http://10.87.10.32:' + portNumber + '/notification'
    print portTest
    port = loc.getTranslationNodeNotificationPortType(portTest, tracefile=sys.stdout)

  # actualy ask the service to do the job
    resp = port.chunkIsReady(req)
