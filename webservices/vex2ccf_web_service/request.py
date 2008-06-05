#!/usr/bin/env python2.4
import sys
from vex2ccf_services import *

portNumber = 8081;
portTest = 'http://192.42.120.69:'+str(portNumber)+'/vex2ccf'
portTest = 'http://localhost:'+str(portNumber)+'/vex2ccf'

if __name__ == "__main__":
  if len(sys.argv) != 2:
    print "%s takes a vex-file as argument" % sys.argv[0]
    sys.exit(1)

  loc = Vex2ccfLocator()
  port = loc.getvex2ccf_port_type(portTest)

  request = convertRequest();

  filehandle = open(sys.argv[1])

  request._vex_file = filehandle.read()

  response = port.convert(request);

  print response._ccf_file
