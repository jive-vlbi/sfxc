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

inputdirectory = "/home/damien/code/sfxc_25marc/sfxc/stats/"
outputdirectory = "output_dir/"



def parse_name(string, totalfilename):
  r = {}
  m = re.match(".*?\[(.*?)\].*", string)
  if m:
    print "T: "+m.group(1)
    r[m.group(1)] = parse_name( re.compile("\["+m.group(1)+"\]").sub("",string), totalfilename ) 
    return r
  m = re.compile("\.data").sub("", totalfilename)
  if m:
    return {"data" : m }
  return {"data" : "No file name"}

def parse_prefix(string,totalfilename):
  r = {}
  m = re.match("(.*?)\[.*", string)
  if m:
    print "Prefix: "+m.group(1)
    r[m.group(1)] = parse_name( re.compile(m.group(1)).sub("",string), totalfilename ) 
    return r  
  m = re.compile("\.data").sub("", totalfilename)
  if m:
    return {"data" : m }
  return {"data" : "No file name"}
 

def mergedict(out, in2):
  keys = in2.keys()
  for i in keys:
    if out.has_key(i):
      mergedict(out[i], in2[i])
    else:
      out[i]=in2[i]  

def gen_picture(indir, filename, outdir):
  print " building picture for: " +indir+""+ filename 
  drawer.do_picture(indir+filename+".data", outdir+filename+".png")
  return filename+".png"


def flatten_dict(datablock):
  if isinstance(datablock, dict):
    res = []
    for i in datablock.keys():
      ret = flatten_dict(datablock[i])
      res += ret[:]
    return res
  return [datablock]
  
def do_page(pagename, datablock, destdir):
  print "Creating page:"+pagename
  fout = open(destdir+pagename+".html", "w")
  fout.write("<html><head></head><body>")
  fout.write("<table>")    
  
  data = flatten_dict(datablock)
  for i in data:
    print " Image: " + destdir+i+".png"
    fout.write('<tr><td>'+i+'</td> <td><img src="'+i+'.png" width=2000/></td></tr>')    
  
  fout.write("</table>")
  fout.write("</body></html>")      
  fout.close()  
  
    
   
def rec_index(indir, data, outdir):
  if isinstance(data, dict) and len(data.keys()) == 1 and data.has_key("data"):
    picturename = gen_picture(indir, data["data"], outdir )
    retstr = '<a href="'+picturename+'"> '+`data.values()`+'</a>\n'
    return [True, retstr]

  if isinstance(data, dict):
    if len( data.keys() ) == 1:
      i = data.keys()[0]
      val = rec_index(indir, data[i], outdir)
      retstr = ""
      if val[0] == True:
        #retstr += '<LI><a href=""> ['+i+']</a>\n'
        #retstr += '<UL>'
        retstr += val[1]
        #retstr += '</UL>'
        #retstr += '</LI>'
        return [False, retstr]
      else:
        #retstr += '<LI><a href=""> ['+i+']</a>\n'
        retstr += val[1]
        #retstr += '</LI>'
        return [True, retstr]
    else:     
      retstr = "<UL>"
      for i in data.keys():
        print "DEBUG"
        val = rec_index(indir, data[i], outdir)
        if val[0] == True:
          #retstr += '<LI><a href=""> ['+i+']</a>\n'
          #retstr += '<UL>'
          retstr += val[1]
          #retstr += '</UL>'
          #retstr += '</LI>'
        else:
          retstr += '<LI><a href=""> '+i+'</a>\n'
          retstr += val[1]
          retstr += '</LI>'
        print "Adding new Index=>" + i
      retstr+="</UL>"
      return [False, retstr]
  return None
   
def gen_index(indir, data,  outputdir):
  index = open(outputdir+"/index.html", "w")
  index.write("<html><head></head><body>")
  index.write("<UL>")
  for i in data.keys():
    print "DEBUG"
    index.write('<LI><a href=""> '+i+'</a></LI>\n')
    ret = rec_index(indir, data[i], outputdir)
    if ret[0] == True:
      index.write( ret[1] )
    else:
      index.write( ret[1] )      
    print "Adding new Index=>" + i
  index.write("</UL>")
  index.write("</body></html>")
  

data = {}
for i in os.listdir(inputdirectory):
  #print "Test:" + `parse_prefix(i)`
  mergedict(data, parse_prefix(i, i))
  #print "Total dict: "+`data`

gen_index(inputdirectory, data, outputdirectory)

for i in data.keys():
  do_page(i, data[i], outputdirectory)
