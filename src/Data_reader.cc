/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Data_reader.h>

Data_reader::Data_reader() : _data_counter(0) {
}
  
Data_reader::~Data_reader() {
}
  
size_t
Data_reader::get_bytes(size_t nBytes, char *buff) {
  size_t result = do_get_bytes(nBytes, buff);
  _data_counter += result;
  return result;
}
  
UINT64 
Data_reader::data_counter() {
  return _data_counter;
}

void 
Data_reader::reset_data_counter() {
  _data_counter = 0;
}
