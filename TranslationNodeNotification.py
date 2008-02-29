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

  # define request
    req = chunkIsReadyRequest()
    req.Param0 = req.new_param0()  

  # get broker ip address from the read file
    req.Param0.BrokerIPAddress = BrokerIPAddress
    req.Param0.chunkId = chunkId
    req.Param0.chunkLocation = chunkLocation
    req.Param0.chunkSize = chunkSize
    req.Param0.endTime = endTime
    req.Param0.startTime = startTime
    req.Param0.translationNodeIP = translationNodeIP
    req.Param0.translationNodeId = translationNodeId


    print 'chunk ID: ', req.Param0.chunkId
    print 'chunk Location: ', req.Param0.chunkLocation
    print 'Requested chunk size: ', req.Param0.chunkSize
    print 'Requested end time: ', req.Param0.endTime
    print 'Requested start time: ', req.Param0.startTime
    print 'translationNode IP: ', req.Param0.translationNodeIP
    print 'translationNode Id: ', req.Param0.translationNodeId

  # get port number from the read file
    portNumber_string = "8080"
    portNumber = portNumber_string.strip()
    print portNumber

  # test if the http address correctly parsed
    loc = TranslationNodeNotificationLocator()
    portTest = 'http://localhost:' + portNumber + '/test'
    print portTest
    port = loc.getTranslationNodeNotificationPortType(portTest)

  # actualy ask the service to do the job
    resp = port.chunkIsReady(req)
