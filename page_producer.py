import gc


import numarray as na
import numarray.ieeespecial as ieee
from pylab import *

import logic_user
from logic_user import *

import drawer

def qMonitor(entryname, Monitor=UnifyingVariable() ):
  for i in query(Subject=Monitor, Predicate=UnifyingVariable(), Object="monitor_speed"): 
    for j in query(Subject=UnifyingVariable(), Predicate="has", Object=i[0]):
      for k in query(Subject=entryname, Predicate="has", Object=j[0]):
        yield [k[0], i[0]]


def write_semantic_info(fout, entrynode):
  fout.write("<hr>")
  fout.write("semantic info:<br/>")
  for i in query(Subject=entrynode, Predicate=UnifyingVariable(), Object=UnifyingVariable()):
    #print "Entry:" + `i`
    fout.write(""+i[0]+" "+i[1]+" "+`i[2]`+"<br/>")  
   
  for i in query(Subject=entrynode, Predicate=UnifyingVariable(), Object=UnifyingVariable()): 
    for j in query(Subject=i[2], Predicate=UnifyingVariable(), Object=UnifyingVariable()):
      fout.write(""+j[0]+" "+j[1]+" "+`j[2]`+"<br/>")  
      for k in query(Subject=j[2], Predicate=UnifyingVariable(), Object=UnifyingVariable()):
        fout.write(""+k[0]+" "+k[1]+" "+`k[2]`+"<br/>")  
             
  fout.write("<hr>")

#
# This function create a reduced 
# version of the monitoring information
# and make htmllink for each of its part
# 
def create_miniature(monitorname, bounds):
  global database
  for i in query(Subject="gen-runtime", Predicate="cacheddir", Object=UnifyingVariable()):
    database.append([monitorname ,"largepicture", i[2]+monitorname+".png"])
    database.append([monitorname ,"largepicture_url", monitorname+".png"])
    database.append([monitorname ,"minipicture", i[2]+monitorname+"_thumbails.png"])
    database.append([monitorname ,"minipicture_url", monitorname+"_thumbails.png"])

  g = inputdir().next()
  
  for i in query(monitorname, Predicate="location", Object=UnifyingVariable()):
    large_picture=query(monitorname, Predicate="largepicture", Object=UnifyingVariable()).next()[2]
    large_picture_url=query(monitorname, Predicate="largepicture_url", Object=UnifyingVariable()).next()[2]
    small_picture=query(monitorname, Predicate="minipicture", Object=UnifyingVariable()).next()[2]
    small_picture_url=query(monitorname, Predicate="minipicture_url", Object=UnifyingVariable()).next()[2]
    typeofmon = query(monitorname, "is_a", Object=UnifyingVariable()).next()
    
    
    if re.search("_state", typeofmon[0]):
      print "Do a picture from state:"+i[2]+" to "+large_picture
      drawer.do_picture_raw(g+i[2], large_picture, bounds)
      drawer.rescale(large_picture,  small_picture) 
    else:
      print "Do a picture from speed:"+i[2]+" to "+large_picture
      drawer.do_picture(g+i[2], large_picture, bounds)
      drawer.rescale(large_picture,  small_picture)   
  
class Contains():
  def __init__(self, a, b):
      self.a = a
      self.b = b
  
  def check(self):
    if isinstance(self.a, UnifyingVariable):
      return re.search(self.b, self.a._value)
    else:
      return re.search(self.b, self.a)
    
class NotContains():
  def __init__(self, a, b):
      self.a = a
      self.b = b
  
  def check(self):
    if isinstance(self.a, UnifyingVariable):
      return not re.search(self.b, self.a._value)
    else:
      return not re.search(self.b, self.a)    
    
def match(a, b):
  for i in a:
    if b.check():
      yield i  
      
      
def write_miniature_state(fout, entryname, bounds):
  fout.write("<hr>")
  
  mName = UnifyingVariable()
  mP = UnifyingVariable() 
  b = []
  for i in  match( qMonitor(entryname, mName), Contains(mName, "_state") ):
    for j in query(i[1], "loading_percentage", mP):
      b.append( [ i[0], i[1], j[-1] ] )
  
  b.sort(reverse=True, key=operator.itemgetter(2)) 
  for j in b:
      #print "TPROUT ?"+`j`
      create_miniature(j[1], bounds)
      fout.write(j[1]+": "+ ("%0.2f" % j[2])+"%")
      
      pngname = query(j[1], "minipicture_url", Object=UnifyingVariable()).next()
      bigpngname =  query(j[1], "largepicture_url", Object=UnifyingVariable()).next()
      fout.write("<a href='"+bigpngname[2]+"'><img src='"+pngname[2]+"'  width=1024/></a>")
        
  fout.write("</hr>")



def write_miniature_speed(fout, entryname, bounds):
  fout.write("<hr>")
  
  mName = UnifyingVariable()
  mP = UnifyingVariable() 
  b = []
  for i in  match( qMonitor(entryname, mName), NotContains(mName, "_state") ):
    for j in query(i[1], "avreage_speed", mP):
      create_miniature(i[1], bounds)
      fout.write(i[1]+": "+ ("%0.2f" % j[2])+" MB/sec")
      
      pngname = query(i[1], "minipicture_url", Object=UnifyingVariable()).next() 
      #bigpngname =  query(j[1], "largepicture_url", Object=UnifyingVariable()).next()
      bigpngname =  query(i[1], "largepicture_url", Object=UnifyingVariable()).next()
      fout.write("<a href='"+bigpngname[2]+"'><img src='"+pngname[2]+"'  width=1024/></a>")
        
  fout.write("</hr>")

# for each of the monitor we should get the timming information
# to print unifyied graphs.   
def find_bounds(entryname):
  global database
  mintime = ieee.plus_inf
  maxtime = 0
  
  g = inputdir().next()
  for i in qMonitor(entryname):
    datafile = query(i[1], "location", Object=UnifyingVariable()).next()[2]
    print "COUCOU: "+`g+datafile`
    array = load(g+datafile)
    begintime = array[:,0]
    endtime   = array[:,1]
    
  
    if mintime > begintime[0]:
      mintime = begintime[0]
    if maxtime < endtime[-1]:
      maxtime = endtime[-1]

  database.append([entryname, "starttime", mintime])
  database.append([entryname, "stoptime", maxtime])
  return [mintime, maxtime]
  

def compute_monitor_statistics(monitorname, bounds):
  global database
  g = inputdir().next()
  
  #print "Statistic for: " + monitorname
  datafile = query(monitorname, "location", Object=UnifyingVariable()).next()[2]
  #print "Data file is: " + g + datafile
  array = load(g+datafile)
  begintime = array[:,0]
  endtime   = array[:,1]
  realtime   = array[:,2]
  value     = array[:,3]
  
  totalduration = bounds[-1]-bounds[0]
  sumduration = 0
  sumvalue = 0
  for i in range(0, len(begintime)):
      sumduration += realtime[i]
      sumvalue += realtime[i]*value[i]/(endtime[i]-begintime[i])
    
  database.append([monitorname, "sumduration", sumduration/1000000]) 
  database.append([monitorname, "sumvalue", sumvalue]) 
  database.append([monitorname, "totalduration", totalduration/1000000]) 
  database.append([monitorname, "loading_percentage", sumduration*100/totalduration]) 
  database.append([monitorname, "avreage_speed",  (sumvalue/(1024*1024))/(sumduration/1000000)]) 
   
def compute_statistics(entryname, bounds):
  for i in qMonitor(entryname):
    compute_monitor_statistics(i[1], bounds)    
    
def produce_page(entryname, filename):
  print "Creating page: "+filename
  fout=open(filename, "w")
  fout.write("""
  <html>
    <head></head>
    <body>
  """)
  fout.write("<center><h1>"+entryname+"</h1></center>") 
  bounds = find_bounds(entryname) 
  compute_statistics(entryname, bounds)
  write_miniature_state(fout, entryname, bounds)
  write_miniature_speed(fout, entryname, bounds)    
  write_semantic_info(fout, entryname)  
    
  fout.write("""
    </body>
    </html>
  """)
  
  

