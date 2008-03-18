import sys
from TranslationNodeNotification_services import *

# define request
req = chunkIsReadyRequest()
req.Param0 = req.new_param0()

# get broker ip address from the read file
# actually both declaration types work
# declaring i.e. req.chunkId=0 wouldn't work
req.Param0.ChunkId = 10001
req.Param0.set_element_chunkLocation("huygens/data4/")
req.Param0.set_element_chunkSize(123456)
req.Param0.set_element_endTime("2007y158d18h59m00s")
req.Param0.set_element_startTime("2007y158d18h56m00s")
req.Param0.set_element_translationNodeIP("192.42.120.22")
req.Param0.set_element_translationNodeId(20001)

# get port number from the read file
portNumber_string = "8080"
portNumber = portNumber_string.strip()
print portNumber

# test if the http address correctly parsed
# get a port proxy instance
loc = TranslationNodeNotificationLocator()
portTest = 'http://jop32:' + portNumber + '/notification'
print portTest
port = loc.getTranslationNodeNotificationPortType(portTest, tracefile=sys.stdout)

# Note that both of the following implementations work
print "Chunk Id", req.Param0.get_element_chunkId()
print "Chunk Location", req.Param0.ChunkLocation

# actualy ask the service to do the job
resp = port.chunkIsReady(req)
