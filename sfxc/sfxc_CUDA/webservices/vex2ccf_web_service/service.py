#!/usr/bin/env python2.4
import sys, re, time, tempfile, os
from ChunkedServiceContainer import AsServer
from vex2ccf_services_server import *

import vex2ccf

portNumber = 8081;

class Service(Vex2ccf):

  def soap_convert(self, ps):
    response = Vex2ccf.soap_convert(self, ps)
    vex_file = self.request._vex_file

    filename = tempfile.mktemp()
    
    filehandle = open(filename, 'w')
    filehandle.write(vex_file)
    filehandle.write('\n')
    filehandle.close()
    
    try:
      ccf = vex2ccf.vex2ccf(filename)
      response._ccf_file = ccf
    except:
      print "Error in parsing"
      response._ccf_file = ""

    os.remove(filename)
    
    # Return the response
    return response

if __name__ == "__main__" :
  port = portNumber
  AsServer(port, (Service('vex2ccf'),))
