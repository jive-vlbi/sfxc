#! /usr/bin/python 

import cgitb; cgitb.enable()
import math

fp = open("../tmpdata/status_file_json.txt", "rt")
#fp = open("status_file_json.txt", "r")

#print "Content-Type: text/json"
print "Content-Type: text/html"
print ""
#print "<html>Hello</html>"
print fp.read()

  
#fp.close()


