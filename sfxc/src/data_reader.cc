/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "utils.h"
#include "data_reader.h"
#include <stdlib.h>
#include <string.h>
#include <limits>

Data_reader::Data_reader() : _data_counter(0), data_slice(-1), is_seekable_(false) {}

Data_reader::~Data_reader() {}

size_t
Data_reader::get_bytes(size_t nBytes, char *buff) {
  SFXC_ASSERT((data_slice==-1) || (nBytes <= (size_t)data_slice));
  // Read at most max_int bytes:
  //  otherwise we can't return the number of bytes read
  const size_t max_int = (size_t)(std::numeric_limits<int>::max());
  if (nBytes > max_int) nBytes = max_int;

  size_t result = do_get_bytes(nBytes, buff);
  _data_counter += result;
  if (data_slice != -1) data_slice -= result;
  return result;
}

uint64_t
Data_reader::data_counter() {
  return _data_counter;
}

void
Data_reader::reset_data_counter() {
  _data_counter = 0;
}

void
Data_reader::set_size_dataslice(int data_size) {
  SFXC_ASSERT(data_size >= -1);
  data_slice = data_size;
}

int
Data_reader::get_size_dataslice() {
  return data_slice;

}

bool
Data_reader::end_of_dataslice() {
  return (data_slice == 0) || eof();
}
