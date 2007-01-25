/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#include <Data_writer_file.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include <fcntl.h> // file control

Data_writer_file::Data_writer_file(const char *filename) : 
  Data_writer()
{
  file = fopen64(filename, "wb");
  assert(file != NULL);
}

Data_writer_file::~Data_writer_file() {
  fclose(file);
}
  
UINT64 
Data_writer_file::put_bytes(UINT64 nBytes, char *buff) {
  UINT64 result = fwrite(buff, 1, nBytes, file);
  assert(result == nBytes);
  return result;
}
