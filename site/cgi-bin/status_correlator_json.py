#! /usr/bin/python 

###############################################################
# 
#  This script return the correlator state as a json structure.
#  The structure is the following:
# 
#  
#
#  Damien Marchal (UvA), Copyright 2008 
# 
################################################################

import cgitb; cgitb.enable()
import math

# printing the content type followed by a empty line
print "Content-Type: text/html"
print ""

# printing the content of the filee
#print """
#{
#  correlator_state : 
#  {
#     code : 0,
#     desc : "Not running",
#  }
#}
#"""
print """
{
  correlator_state : 
  {
     code : 0,
     desc : "No ddd running",
     curr_state : 0     
  }
}
"""

