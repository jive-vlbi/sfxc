from TranslationNode_services import *
# we need to skip comments in input file.
# For that we make use of re
import re

# define comment characters
comment_char = re.compile(r'^\s*#')

# define a list and initialize it with anything and don't use inp[0] later on 
inp = ['##']
# if the first character matches the comment character skip that line
# otherwise read the line
fileExp = file('experiment_data.inp')
for line in fileExp:
  if comment_char.match(line):
    continue
  inp.append(line)

# we are done with reading. we can close the file
fileExp.close()

# define request
req = startTranslationJobMessage()
req.Param0 = req.new_param0()  

# get broker ip address from the read file
req.Param0.BrokerIPAddress=inp[1].strip()
print req.Param0.BrokerIPAddress

# get chunk size to read in from the read file
bytes_string = inp[2]
bytes = bytes_string.strip()
print bytes

# get port number from the read file
portNumber_string = inp[3]
portNumber = portNumber_string.strip()
print portNumber

# test if the http address correctly parsed
loc = TranslationNodeLocator()
portTest = 'http://localhost:' + portNumber + '/test'
print portTest
port = loc.getTranslationNodePortType(portTest)

# initialize the chunk size
if (bytes == "scan size"):
  req.Param0.ChunkSize=0
else: 
  req.Param0.ChunkSize=eval(bytes) 

print req.Param0.ChunkSize

# read the rest of the parameters from the read file
req.Param0.StartTime=inp[4].strip()
req.Param0.EndTime=inp[5].strip()
req.Param0.StationName=inp[6].strip()
req.Param0.ExperimentName=inp[7].strip()

print req.Param0.StartTime
print req.Param0.EndTime
print req.Param0.StationName
print req.Param0.ExperimentName

# actualy ask the service to do the job
resp = port.startTranslationJob(req)
