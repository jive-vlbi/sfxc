import sys

class Incr_file:
  def __init__(self, filename):
    self.seek = 0
    self.filename = filename
    if filename == sys.stdin or filename==None:
      self.fp = sys.stdin
    else:
      self.fp = open(filename, "r")
  

  def read(self):
    while True:
      if not self.fp == sys.stdin:
        self.fp = open(self.filename, "r")
        self.fp.seek(self.seek)
        
      for i in self.fp:
        yield i
        
      if not self.fp == sys.stdin:
        self.seek = self.fp.tell()
        self.fp.close()
