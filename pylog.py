#! /usr/bin/env python 

###########################################
#
# Scan a directory containing the different
# runtime statistic of an execution.
# 
# Author: Damien Marchal
# Copyright 2008 Univesity of Amsterdam
# 
##########################################
import logic
from logic import *

database = []

dpredicate = {}
dsubject = {}
dobject = {}

def add_in_database(entry):
  global database
  global dpredicate
  global dsubject
  global dobject
  
  database.append(entry)
  predicatedb = "database_"+entry[1]
  stringdb ="try:\n\tif "+predicatedb+":\n\t\tNone\nexcept NameError:\n\t"+predicatedb+"=[]\n"+predicatedb+".append("+`entry`+")"

  exec stringdb in globals()
  
  string = "def "+entry[1]+"(Subject=UnifyingVariable(), Object=UnifyingVariable()):"+"""
  global database_"""+entry[1]+"""
  for i in  """+predicatedb+""":
    for j in generalUnify(Subject, i[0]):
      for k in generalUnify(Object, i[2]):
        yield [i[0], i[2]]
"""
  exec string in globals()
  
  string = "class Class_"+entry[0]+":"+"""
  def """+entry[1]+"""(self, Object=UnifyingVariable()):  
    global """+predicatedb+"""
    for i in """+predicatedb+""":
      for k in generalUnify("""+entry[0]+""", i[0]):
        for j in generalUnify(Object, i[2]):
          yield [i[2]]    
"""+entry[0]+"=Class_"+entry[0]+"()"  
  exec string in globals()

    

add_in_database(["node", "has_instance", "node211"])
add_in_database(["node", "has_instance", "node212"])
add_in_database(["node", "has_instance", "node213"])

add_in_database(["node211", "is_a", "node"])
add_in_database(["node212", "is_a", "node"])
add_in_database(["node213", "is_a", "node"])

add_in_database(["node211", "has", "node211_CPU0"])
add_in_database(["node212", "has", "node212_CPU0"])
add_in_database(["node213", "has", "node213_CPU0"])

add_in_database(["node211_CPU0", "is_a", "cpu"])
add_in_database(["node212_CPU0", "is_a", "cpu"])
add_in_database(["node213_CPU0", "is_a", "cpu"])


for i in has():
  print "has: "+`i`

for i in node211.has():
  print "all "+`i`

for i in node.has_instance():
  for j in eval(i[0]).has():
    print "node has_instance "+`i[0]`+" has "+`j`


