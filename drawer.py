import gc

import matplotlib
matplotlib.use('Agg')

import numarray as na
import Image

from pylab import *
matplotlib.patches.lines.antialiased = False
matplotlib.patches.antialiased = False


def doCpuPicture(filename):
    X = load(filename+".dat")

    begintime = X[:,0]
    user     = X[:,1]
    nice     = X[:,2]
    system     = X[:,3]
    idle     = X[:,4]
    starttime = begintime[0]
    
    for i in range(0, len(begintime)):
		begintime[i] = (begintime[i] - starttime) 
		
    for i in range(0, len(begintime)-1 ):
        bottom = 0
        weidth = begintime[i+1]-begintime[i]
        bar( begintime[i], user[i], width= weidth, color='b', bottom=0)
        bottom += user[i]
        bar( begintime[i], system[i], width= weidth,  color='g', bottom=bottom)
        bottom += system[i]
        b1 = bar( begintime[i], nice[i], width= weidth,color='y', bottom=bottom)
        bottom += nice[i]
        b2 =bar( begintime[i], idle[i], width= weidth, color='r', bottom=bottom)

def itrange(bg, end):
  for i in range(bg, end):
    yield i


def do_picture_raw(filename, outputfile, bounds):
    f = figure(figsize=[37, 4.0])  
    ticksPerSec = 1000000
    gca().set_xlim([0, (bounds[1]-bounds[0])/ticksPerSec])
    Y = load(filename)
    
    begintime = Y[:,0]
    endtime   = Y[:,1]
    realduration = Y[:,2]
    values = Y[:,3]
    
    title("Plot for ["+filename+"] between: "+`[0, (bounds[1]-bounds[0])/ticksPerSec]`)
    print("Plot for ["+filename+"] between: "+`[0, (bounds[1]-bounds[0])/ticksPerSec]`)
    for i in range(1, len(begintime)):
        begintime[i] = (begintime[i] - bounds[0])/ticksPerSec
        endtime[i] = (endtime[i] - bounds[0])/ticksPerSec
        duration = endtime[i]-begintime[i] 
        
        realduration[i] = realduration[i]/ticksPerSec 
        values[i] = (100*realduration[i]/duration)
        
        bar( begintime[i], values[i], width=duration, linewidth=0.0000, bottom=0)    
       
    gca().set_xlim([0, (bounds[1]-bounds[0])/ticksPerSec])
    gca().set_ylim([0, 100])
    savefig(outputfile, dpi=100)
 
 
def do_picture2(filename, outputfile, bounds):
    print "BEGIN IMAGE2:"+`len(gc.get_objects())`
    f = figure(figsize=[37, 4.0])
    
    print "STEP 2:"+`len(gc.get_objects())` 
  
    ticksPerSec = 1000000
    gca().set_xlim([0, (bounds[1]-bounds[0])/ticksPerSec])
    Y = load(filename)
    print "STEP 3:"+`len(gc.get_objects())` 
  
    
    begintime = Y[:,0]
    endtime   = Y[:,1]
    realduration = Y[:,]
    
    #value     = Y[:,2]	
    
    title("Plot for ["+filename+"] between: "+`[0, (bounds[1]-bounds[0])/ticksPerSec]`)
    print("Plot for ["+filename+"] between: "+`[0, (bounds[1]-bounds[0])/ticksPerSec]`)
    #begintime[0] = (begintime[0] - bounds[0])/ticksPerSec
    #endtime[0] = (endtime[0] - bounds[0])/ticksPerSec
    #value[0] = 100 
    #dsta=[]
    #dstv=[]   
    #for i in range(1, len(begintime)):
        #begintime[i] = (begintime[i] - bounds[0])/ticksPerSec
        #endtime[i] = (endtime[i] - bounds[0])/ticksPerSec
        #weidth = endtime[i]-begintime[i]
        #value[i] = weidth/(endtime[i]-endtime[i-1])*100
        
        #dsta.append( endtime[i-1] )
        #dstv.append( value[i] )
        #dsta.append( begintime[i] )
        #dstv.append( value[i] )
        
        
    

    #idxbg = 0
    #idxnd = 0
    #zero = 0
    #sumduration = endtime[0]-begintime[0]
    #number = 0
    #for i in range(0, len(begintime)):
    #   closeness = endtime[i]-endtime[i-1]
    #   if closeness == 0:
    #    zero += 1
    #   if closeness < 0.008:
    #    number += 1
      
    #print "Number are :" +`zero`+" other values:"+`number`+" total elements:"+`len(begintime)`
    #value[0] = 0
    i=0
    end = len(begintime)
    while i < end:
    
    
      duration = 0
      sumduration = 0
      idb = i
      same = True
      olddur = None
      
      while duration<0.1 and i < end:
          begintime[i] = (begintime[i] - bounds[0])/ticksPerSec
          endtime[i] = (endtime[i] - bounds[0])/ticksPerSec
     
          sumduration += endtime[i]-begintime[i]
          duration = endtime[i]-begintime[idb]
          i+=1
      
      i+=1
      #if sumduration/duration > 0.05:   
      bar( begintime[idb], sumduration/duration, width=duration, linewidth=0.0000, bottom=0)  
   
    #for i in range(0, 200):
    #  bar( i, 1, width=0.1, linewidth=0.0000, bottom=0, color='r')       
    gca().set_xlim([0, (bounds[1]-bounds[0])/ticksPerSec])
    savefig(outputfile, dpi=100)
    print "STEP 5:"+`len(gc.get_objects())` 
    del Y
    del f
    del begintime
    del endtime
    print "END IMAGE2:"+`len(gc.get_objects())`
  
    

def do_picture(filename, outputfile, bounds):
    f =figure(figsize=[37, 4.0])
    print "STEP 2:"+`len(gc.get_objects())` 
   
    ticksPerSec = 1000000
    gca().set_xlim([0, (bounds[1]-bounds[0])/ticksPerSec])
    Y = load(filename)
    begintime = Y[:,0]
    endtime   = Y[:,1]
    value     = Y[:,2]	
    
    title("Plot for ["+filename+"] between: "+`[0, (bounds[1]-bounds[0])/ticksPerSec]`)
    print("Plot for ["+filename+"] between: "+`[0, (bounds[1]-bounds[0])/ticksPerSec]`)
   
    i=0
    end = len(begintime)
    while i < end:
      duration = 0
      sumduration = 0
      idb = i
      sumvalue = 0
      while duration<0.5 and i < end:
          begintime[i] = (begintime[i] - bounds[0])/ticksPerSec
          endtime[i] = (endtime[i] - bounds[0])/ticksPerSec
      
          sumvalue += value[i]
          sumduration += endtime[i]-begintime[i]
          duration = endtime[i]-begintime[idb]
          i+=1
      
      i+=1
      if sumduration/duration > 0.05:   
        bar( begintime[idb], (sumvalue/(1024*1024))/sumduration, width=duration, linewidth=0.0000, bottom=0)  
    
    gca().set_xlim([0, (bounds[1]-bounds[0])/ticksPerSec])  
    savefig(outputfile, dpi=100)
    del Y
    del f
    del begintime
    del endtime
    del value
    print "END IMAGE:"+`len(gc.get_objects())`
  
       
       
def rescale(infilename, outfilename):
  im = Image.open(infilename)
  im.resize((1024, 75)).crop((120,0,850,75)).save(outfilename, "PNG")

       
#f.set_edgecolor('y')      
#do_picture('input_dir/inputnode[3][channel_extractor][working].data', 'output_dir')
#show()

