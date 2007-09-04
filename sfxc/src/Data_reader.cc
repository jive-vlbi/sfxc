/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Data_reader.h>
#include <assert.h>
#include <utils.h>

Data_reader::Data_reader() : _data_counter(0), data_slice(-1) {
}
  
Data_reader::~Data_reader() {
}
  
size_t
Data_reader::get_bytes(size_t nBytes, char *buff) {
  assert((data_slice==-1) || (nBytes <= (size_t)data_slice));
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
  assert(data_size >= -1);
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
