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

Data_writer_file::Data_writer_file(char *filename) : 
  Data_writer(), file(-1)
{
  file = open(filename, O_WRONLY | O_CREAT, 0600);
  assert(file >= 0);
}

Data_writer_file::~Data_writer_file() {
  close(file);
}
  
UINT64 
Data_writer_file::put_bytes(UINT64 nBytes, char *buff) {
  return write(file, buff, nBytes);
}
