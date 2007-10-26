from TranslationNode_services import *


inp = open('data.inp', 'r')
req = startTranslationJobMessage()
req.Param0 = req.new_param0()
req.Param0.BrokerIPAddress=inp.readline().strip()
bytes_string = inp.readline()
bytes = bytes_string.strip()
print bytes

portNumber_string = inp.readline()
portNumber = portNumber_string.strip()

loc = TranslationNodeLocator()
portTest = 'http://localhost:' + portNumber + '/test'
print portTest
port = loc.getTranslationNodePortType(portTest)

if (bytes == "scan size"):
  req.Param0.ChunkSize=0
else: 
  req.Param0.ChunkSize=eval(bytes) 

print req.Param0.ChunkSize

req.Param0.StartTime=inp.readline()
req.Param0.EndTime=inp.readline()
req.Param0.StationName=inp.readline()
req.Param0.ExperimentName=inp.readline()

resp = port.startTranslationJob(req)
