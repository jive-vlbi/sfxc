/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Multiple_data_readers_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */

#include <Data_reader_buffer.h>
#include <assert.h>

Data_reader_buffer::Data_reader_buffer(Buffer *buff)
 : Data_reader(), 
   buffer(buff), bytes_left(0), 
   end_of_file(false)
{
  assert(buffer != NULL);
  data_start = buffer->consume(bytes_left).buffer();
}

Data_reader_buffer::~Data_reader_buffer() {
}

UINT64 Data_reader_buffer::get_bytes(UINT64 nBytes, char *output_buffer) {
  char *out = output_buffer; 
  UINT64 bytes_to_read = nBytes;
  while (bytes_to_read > 0) {
    if (bytes_left == 0) {
      buffer->consumed();
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
      data_start += curr_read;
    }
    bytes_to_read -=curr_read;
    bytes_left -=curr_read;
  }
  return nBytes;
}

bool Data_reader_buffer::eof() {
  return (end_of_file && (bytes_left == 0)); 
}
