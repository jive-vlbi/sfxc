import socket, telnetlib

class Mark5_set_position:
	def __init__(self):
		self.resp_list=[]
		
	def bytes_starting_position(self, host=None,  port = None, portMark5 = None, ipMark5 = None, startPosition = None):
		s = socket.socket()
		print host
		s.bind((host, port))
		s.listen(1)

		""" The IP address of Mark5 starts from 10.88.0.50 and up"""

		tn = telnetlib.Telnet(ipMark5, portMark5)
		
		tn.write("play=off:" + str(startPosition) + ";\n")
		self.resp=tn.read_until("\n")
		tn.write("data_check?;\n")
		self.resp=tn.read_until("\n")
		self.resp_list=self.resp.split(" : ")
		print "length of responce list = ", len(self.resp_list)
		print "offset ==> ", self.resp_list

		s.close()
		return self.resp_list[3]

class Mark5_get_chunks:
	def __init__(self, host=None, port = None, portMark5 = None, ipMark5 = None, fileName = None, blockSize = None, chunkSize = None, startPosition = None):
		
		f = open(fileName, "w")

		print host

		s = socket.socket()
		s.bind((host, port))
		s.listen(1)

		""" The IP address of Mark5 starts from 10.88.0.50 and up"""

		tn = telnetlib.Telnet(ipMark5, portMark5)

		#tn.write("position?\n")
		#resp = tn.read_until("\n")
		#print resp.strip()

		tn.write("disk2net=disconnect\n")
		resp = tn.read_until("\n")
		print resp.strip()
		
		print "disk2net=connect:" + host + ";\n"

		tn.write("disk2net=connect:" + host + ";\n")
		resp = tn.read_until("\n")
		print resp.strip()

		(c, a) = s.accept()
		print a

		tn.write("disk2net=on:" + str(startPosition) + ":" + str(startPosition+chunkSize) + ";\n")
		#tn.write("disk2net=on:0:" + str(start_position+chunk_size) + ";\n")
		resp = tn.read_until("\n")
		print resp.strip()
		
		pos = 0
		while pos < chunkSize:
			data = c.recv(blockSize)
			if not data:
				break
			f.write(data)
			pos += len(data)
			continue

		tn.write("disk2net=disconnect\n")
		resp = tn.read_until("\n")
		print resp.strip()

      	      	f.close()
		c.close()
		s.close()
