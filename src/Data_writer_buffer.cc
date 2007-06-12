/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Data_writer_buffer.h>
#include <assert.h>

Data_writer_buffer::Data_writer_buffer(Buffer *buff)
 : Data_writer(), 
   buffer(buff), bytes_left(0), 
   end_of_file(false)
{
  assert(buffer != NULL);
}

Data_writer_buffer::~Data_writer_buffer() {
}

size_t 
Data_writer_buffer::do_put_bytes(size_t nBytes, char *input_buffer) {
  UINT64 write_bytes = nBytes;
  while (write_bytes > 0) {
    data_start = buffer->produce().buffer();

    UINT32 write_n_bytes = sizeof(value_type);
    if (write_bytes < write_n_bytes) write_n_bytes = write_bytes;

    memcpy(data_start, input_buffer, write_n_bytes);
    
    buffer->produced(write_n_bytes);

    write_bytes -= write_n_bytes;
    input_buffer += write_n_bytes;
  }
  return nBytes;
}
