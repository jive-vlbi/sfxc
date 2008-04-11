/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include <assert.h>
#include <iostream>
#include <algorithm>

#include <fcntl.h> // file control

#include "data_writer_file.h"
#include "utils.h"

Data_writer_file::Data_writer_file(const char *filename) :
    Data_writer() {
  assert(strncmp(filename, "file://", 7)==0);
  file.open(filename+7, std::ios::out | std::ios::binary);
  assert(file.is_open() );
}

Data_writer_file::~Data_writer_file() {
  file.close();
}

size_t
Data_writer_file::do_put_bytes(size_t nBytes, const char *buff) {
  assert(file.good());
  file.write(buff, nBytes);
  if (file.good()) {
//    DEBUG_MSG("Wrote nBytes: "<< nBytes);
    return nBytes;
  }
  return 0;
}

bool Data_writer_file::can_write() {
  DEBUG_MSG("can_write() not yet implemented");
  return true;
}
