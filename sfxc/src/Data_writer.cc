/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Data_writer.h>
#include <utils.h>
#include <assert.h>

Data_writer::Data_writer() : _data_counter(0), data_slice(-1) {
}
  
Data_writer::~Data_writer() {
}
  
size_t
Data_writer::put_bytes(size_t nBytes, char *buff) {
  assert((data_slice==-1) || (nBytes <= (size_t)data_slice));
  size_t result = do_put_bytes(nBytes, buff);
  _data_counter += (int64_t)result;
  data_slice -= result;
  return result;
}
  
uint64_t 
Data_writer::data_counter() {
  return _data_counter;
}

void 
Data_writer::reset_data_counter() {
  _data_counter = 0;
}

void 
Data_writer::set_size_dataslice(int data_size) {
  assert(data_size >= -1);
  data_slice = data_size;
}

int
Data_writer::get_size_dataslice() {
  return data_slice;

}

bool 
Data_writer::end_of_dataslice() {
  return data_slice == 0;
}
