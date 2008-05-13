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
import re, os
import drawer
from sets import Set

import logic_user
from logic_user import *

import page_producer
from page_producer import *

import operator

#inputdirectory = "/home/damien/code/sfxc_11april/sfxc/"
inputdirectory = "/home/damien/code/gen-runtime/test/"
#inputdirectory = "/home/damien/code/gen-runtime/test2/"
#inputdirectory = "/home/damien/das3/home2/scariedm/"
outputdirectory = "site/"
cacheddir = outputdirectory+"cached/"
basecache = "cached/"
debug = open(outputdirectory+"static/debug.html", "w")

database.append(["gen-runtime", "cacheddir", cacheddir])
database.append(["gen-runtime", "inputdir", inputdirectory])

# First load all the filees in the directory
for i in os.listdir(inputdirectory+"stats/"):
  add_to_database(database, inputdirectory+"stats/"+i)
  
# Creating the menu entry and the page. 
for i in sort( inputnode() ):
  database.append( [i[0], "has_main_page", basecache+i[0]+"_mainpage.html"] )
  
for i in sort( correlationnode() ):
  database.append( [i[0], "has_main_page", basecache+i[0]+"_mainpage.html"] )
  

build_stats = True
# produce all html pages.
if build_stats:
  for i in query(Predicate="has_main_page"):
    None
    #cmd = "./page_producer.py "+i[0]+" "+i[1]
    #print "starting: "+cmd
    #os.system(cmd); 
    #produce_page(i[0], outputdirectory+i[2])
  
  
fout = open(outputdirectory+"cached/auto_menubar.js", "w")
fout.write(
"""
//Variables to set
between=28 //The pixel between the menus and the submenus
mainheight=25 //The height of the mainmenus
subheight=22 //The height of the submenus
pxspeed=13 //The pixel speed of the animation
timspeed=15 //The timer speed of the animation
menuy=0 //The top placement of the menu.
menux=0 //The left placement of the menu

//Images - Play with these

level0_regular="images/level0_regular.gif"
level0_round="images/level0_round.gif"
level0_regular="images/barmain.png"
level0_round="images/barmain.png"

level1_regular="images/level1_regular.gif"
level1_round="images/level1_round.gif"
level1_sub="images/level1_sub.gif"
level1_sub_round="images/level1_sub_round.gif"
level1_round2="images/level1_round2.gif"
level2_regular="images/level2_regular.gif"
level2_round="images/level2_round.gif"
bartop="images/bartop.png"
barbottom="images/barbottom.png"
barleft="images/barleft.png"

//Leave this line
preLoadBackgrounds(level0_regular,level0_round,level1_regular,level1_round,level1_sub,level1_sub_round,level1_round2,level2_regular,level2_round, bartop, barbottom, barleft)

//Starting the menu
onload=SlideMenuInit;

// Menu 0
"""
)


fout.write("""
makeMenu('begin')
makeMenu('top', '')
""")

fout.write("""
  makeMenu('top', 'Introduction', 'static/bod_demo.html', 'myFrame')
  makeMenu('top', 'Start experiment', 'static/start.html', 'myFrame')  
  makeMenu('top', 'View processing', 'static/vumeter.html','myFrame' )
  makeMenu('top', 'Runtime statistics')
""")

for i in sort( inputnode() ):
  for j in query(i[0], "has_main_page"):
    fout.write("makeMenu('sub', '"+i[0]+"', '"+j[2]+"','myFrame')\n")

fout.write("""
  makeMenu('top', 'View correlation', 'plotpage/index.html', 'myFrame')
  makeMenu('top', '')
  makeMenu('end')
""")



#fout.write("""
#makeMenu('begin')
#makeMenu('top', '')
#makeMenu('top', 'Runtime statistics')
#""")

#fout.write("makeMenu('sub', 'Real-time plot', 'cached/slices.png','myFrame') \n")

#fout.write("makeMenu('sub', 'Input nodes')\n")
#for i in sort( inputnode() ):
#  for j in query(i[0], "has_main_page"):
#    fout.write("makeMenu('sub2', '"+i[0]+"', '"+j[2]+"','myFrame')\n")

#fout.write("makeMenu('sub', 'Correlation nodes')\n")
#for i in sort( correlationnode() ):
#  for j in query(i[0], "has_main_page"):
#    fout.write("makeMenu('sub2', '"+i[0]+"', '"+j[2]+"','myFrame')\n")
  
#fout.write("""
#makeMenu('top', 'Demo BoD')
#  makeMenu('sub', 'Introduction', 'static/bod_demo.html', 'myFrame')
#  makeMenu('sub', 'View correlation', 'plotpage/index.html', 'myFrame')
#  makeMenu('sub', 'View processing', 'static/vumeter.html','myFrame' )

#makeMenu('top', '')
#makeMenu('end')
#""")


