#! /usr/bin/env python 

###########################################
#
# Produce and visualise the content of a 
# runtime-statistics database.
# 
# Author: Damien Marchal
# Copyright 2008 Univesity of Amsterdam
# 
##########################################
from BaseHTTPServer import HTTPServer
from CGIHTTPServer import CGIHTTPRequestHandler
serveradresse = ("",8080)
server = HTTPServer(serveradresse, CGIHTTPRequestHandler)
server.serve_forever()

