/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Data_reader_buffer.h>
#include <assert.h>

Data_reader_buffer::Data_reader_buffer(boost::shared_ptr<Buffer> buff)
 : Data_reader(), 
   buffer(buff), bytes_left(0), 
   end_of_file(false)
{
  assert(buffer != NULL);
}

Data_reader_buffer::~Data_reader_buffer() {
}

size_t Data_reader_buffer::do_get_bytes(size_t nBytes, char *out) {
  size_t bytes_to_read = nBytes;
  while (bytes_to_read > 0) {
    if (bytes_left == 0) {
      data_start = buffer->consume(bytes_left).buffer();
      if (bytes_left == 0) {
        end_of_file = true;
        return nBytes - bytes_to_read;
      }
    }
    size_t curr_read = (bytes_to_read < (UINT64)bytes_left ? bytes_to_read : bytes_left);
    if (out != NULL) {
      memcpy(out, data_start, curr_read);
      out += curr_read;
    }
    data_start += curr_read;
    bytes_to_read -=curr_read;
    bytes_left -=curr_read;
    
    if (bytes_left == 0) {
      buffer->consumed();
    }
  }
  return nBytes;
}

bool Data_reader_buffer::eof() {
  return (end_of_file && (bytes_left == 0)); 
}
