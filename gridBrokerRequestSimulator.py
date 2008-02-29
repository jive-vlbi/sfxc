from TranslationNodeNotification_services import *

# define request
req = chunkIsReadyRequest()
req.Param0 = req.new_param0()  

# get broker ip address from the read file
req.Param0.BrokerIPAddress="192.42.120.69"
req.Param0.chunkId = 0
req.Param0.chunkLocation = "huygens/data4/"
req.Param0.chunkSize = 123456
req.Param0.endTime = "2007y158d18h59m00s"
req.Param0.startTime = "2007y158d18h56m00s"
req.Param0.translationNodeIP = "192.42.120.22"
req.Param0.translationNodeId = "huygens"


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
