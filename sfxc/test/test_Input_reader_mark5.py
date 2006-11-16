#!python

import sys, os,time;

infile = "/jop51_0/kruithof/data/JIVE-13.del"
infile = "data/input.txt"
outfile = "output.txt"

if os.path.exists(outfile): 
  os.remove(outfile);
if os.path.exists("test_Input_reader_mark5"): 
  os.remove("test_Input_reader_mark5");

if os.system("make test_Input_reader_mark5"):
  sys.exit();

os.system("./test_Input_reader_mark5 "+outfile+"&");
#os.system("Net2file -f "+outfile+"&")

time.sleep(.1)
os.system("File2net -f "+infile)

print " - Performing diff between files"
time.sleep(.1)
os.system("sync; diff "+infile+" "+outfile)

print " - Removing output file"
os.remove(outfile)
