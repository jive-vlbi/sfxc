/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Data_writer_file.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include <fcntl.h> // file control

Data_writer_file::Data_writer_file(const char *filename) : 
  Data_writer()
{
  file = FOPEN(filename, "wb");
  assert(file != NULL);
}

Data_writer_file::~Data_writer_file() {
  fflush(file);
  fclose(file);
}
  
size_t 
Data_writer_file::do_put_bytes(size_t nBytes, char *buff) {
  UINT64 result = fwrite(buff, 1, nBytes, file);
  assert(result == (UINT64)nBytes);
  return result;
}
