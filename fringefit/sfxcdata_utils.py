import time, os
timeslice_header_size = 16
uvw_header_size = 32
stat_header_size = 24
baseline_header_size = 8

class EndOfData(Exception):
  """ If no more data can be read from a correlation file (after timeout) then this exception is thrown""" 
  def __init__(self, value):
      self.value = value
  def __str__(self):
     return repr(self.value)

def read_data(corfile, nbytes, timeout = 0):
  """ Reads nbytes of data the input file, if insufficient data is available then
  the function will wait for atmost timeout seconds"""
  data = corfile.read(nbytes)
  data_read = len(data)
  if data_read != nbytes:
    statinfo = os.stat(corfile.name)
    start_time = statinfo.st_mtime 
    current_time = time.time()
    while ((current_time - start_time) < timeout) and (data_read != nbytes):
      data += corfile.read(nbytes-data_read)
      data_read = len(data)
      time.sleep(1) # Sleep for one second
      current_time = time.time()
    if data_read != nbytes:
      raise EndOfData('EOF')
  return data

def skip_data(corfile, nbytes, timeout = 0):
  """ Skips nbytes from the current position in the input file, if insufficient data is available then
  the function will wait for atmost timeout seconds"""
  statinfo = os.stat(corfile.name)
  previous_statinfo = statinfo
  pos = corfile.tell()
  if (statinfo.st_size - pos) < nbytes:
    start_time = statinfo.st_mtime
    current_time = time.time()
    while ((statinfo.st_size - pos) < nbytes) and ((current_time - start_time) < timeout):
      time.sleep(1)
      current_time = time.time()
      statinfo = os.stat(corfile.name)
      if previous_statinfo != statinfo.st_size:
        # Reset timeout
        start_time = statinfo.st_mtime
        previous_statinfo = statinfo
    if (statinfo.st_size - pos) < nbytes:
      raise EndOfData('EOF')
  corfile.seek(nbytes, 1)
