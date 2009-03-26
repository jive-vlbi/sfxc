import socket, telnetlib

##host = socket.gethostbyname(socket.gethostname())
host = "192.42.120.69"

class Mark5(object):
    def __init__(self, portMark5, ipMark5):
	## The IP address of Mark5 starts from 10.88.0.50 and up 
	self.port = portMark5
	self.ip = ipMark5
    def bytes_starting_position(self, startPosition):
	"Returns a time in vex format, eg., '2008y062d19h30m14.3600s'"
	tn = telnetlib.Telnet(self.ip, self.port)
	tn.write("play=off:" + str(startPosition) + ";\n")
	resp = tn.read_until("\n")
	tn.write("data_check?;\n")
	resp = tn.read_until("\n")
	print resp.strip()
	tn.close()
	resp_list = resp.split(" : ")
	return resp_list[3]
    def get_chunks(self, fileName, blockSize, chunkSize, startPosition):
	f = open(fileName, "w")
	tn = telnetlib.Telnet(ipMark5, portMark5)
	##tn.write("position?\n")
	##resp = tn.read_until("\n")
	##print resp.strip()
	tn.write("disk2net=disconnect\n")
	resp = tn.read_until("\n")
	print resp.strip()
	print "disk2net=connect:" + host + ";\n"
	tn.write("disk2net=connect:" + host + ";\n")
	resp = tn.read_until("\n")
	print resp.strip()	

	tn.write("disk2net=on:" + str(startPosition) + ":" + str(startPosition+chunkSize) + ";\n")
	#tn.write("disk2net=on:0:" + str(start_position+chunk_size) + ";\n")
	resp = tn.read_until("\n")
	print resp.strip()
	
	pos = 0
	while pos < chunkSize:
	    data = c.recv(blockSize)
	    if not data: break
	    f.write(data)
	    pos += len(data)
	
	tn.write("disk2net=disconnect\n")
	resp = tn.read_until("\n")
	print resp.strip()

	f.close()
	tn.close()
    def disconnect(self):
	tn = telnetlib.Telnet(ipMark5, portMark5)
	tn.write("disk2net=disconnect\n")

    
