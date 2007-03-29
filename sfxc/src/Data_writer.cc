/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Multiple_data_writers_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */

#include <Data_writer.h>

Data_writer::Data_writer() : _data_counter(0) {
}
  
Data_writer::~Data_writer() {
}
  
UINT64 
Data_writer::put_bytes(UINT64 nBytes, char *buff) {
  UINT64 result = do_put_bytes(nBytes, buff);
  _data_counter += result;
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
