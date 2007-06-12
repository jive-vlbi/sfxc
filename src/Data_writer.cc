/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Data_writer.h>
#include <iostream>

Data_writer::Data_writer() : _data_counter(0) {
}
  
Data_writer::~Data_writer() {
}
  
size_t
Data_writer::put_bytes(size_t nBytes, char *buff) {
  INT64 result = do_put_bytes(nBytes, buff);
  _data_counter += (INT64)result;
  return result;
}
  
UINT64 
Data_writer::data_counter() {
  return _data_counter;
}

void 
Data_writer::reset_data_counter() {
  _data_counter = 0;
}
