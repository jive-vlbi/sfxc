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
  file.open(filename, std::ios::out | std::ios::binary);
  assert(file.is_open() );
}

Data_writer_file::~Data_writer_file() {
  file.close();
}
  
size_t 
Data_writer_file::do_put_bytes(size_t nBytes, char *buff) {
  assert(file.good());
  file.write(buff, nBytes);
  if (file.good()) return nBytes;
  return 0;
//   uint64_t result = fwrite(buff, 1, nBytes, file);
//   assert(result == (uint64_t)nBytes);
//   return result;
}
