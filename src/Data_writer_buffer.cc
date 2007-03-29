/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Multiple_data_writers_controller.cc 153 2007-02-05 09:12:43Z kruithof $
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

UINT64 Data_writer_buffer::do_put_bytes(UINT64 nBytes, char *input_buffer) {
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
