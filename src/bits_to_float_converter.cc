/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Channel_extractor_mark4.h 219 2007-05-09 11:55:38Z kruithof $
 *
 */
#include "bits_to_float_converter.h"

#include <assert.h>


const FLOAT sample_value_ms[] = {
                                  -7, -2, 2, 7
                                };
const FLOAT sample_value_m[]  = {
                                  -5,  5
                                };

Bits_to_float_converter::Bits_to_float_converter()
    : bits_per_sample(-1), size_output_slice(-1),
output_buffer(Output_buffer_ptr(new Output_buffer(10))) {
  for (int i=0; i<256; i++) {
    lookup_table[i][0] = sample_value_ms[(i>>6) & 3];
    lookup_table[i][1] = sample_value_ms[(i>>4) & 3];
    lookup_table[i][2] = sample_value_ms[(i>>2) & 3];
    lookup_table[i][3] = sample_value_ms[i & 3];
  }
}

void
Bits_to_float_converter::set_parameters(int bits_per_sample_,
                                        int total_size_input_slice_,
                                        int size_output_slice_) {
  if (bits_per_sample_ <= 0) {
    bits_per_sample = bits_per_sample_;
    return;
  }
  if ((bits_per_sample != bits_per_sample_) ||
      (size_output_slice != size_output_slice_)) {
    bits_per_sample = bits_per_sample_;
    size_output_slice = size_output_slice_;
    // One byte contains the offset
    input_char_buffer.resize(size_output_slice/(8/bits_per_sample)+1);
  }

  assert(bits_per_sample_ == 2);
  // Fill the lookup table
  assert(data_reader != Data_reader_ptr());

  if (data_reader->get_size_dataslice() > 0) {
    DEBUG_MSG("size data slice: " << data_reader->get_size_dataslice());
    sleep(1);
    assert(data_reader->get_size_dataslice() <= 0);
  }
  data_reader->set_size_dataslice(total_size_input_slice_);
}

void
Bits_to_float_converter::set_data_reader(boost::shared_ptr<Data_reader> data_reader_) {
  data_reader = data_reader_;
}

Bits_to_float_converter::Output_buffer_ptr
Bits_to_float_converter::get_output_buffer() {
  assert(output_buffer != Output_buffer_ptr());
  return output_buffer;
}

void Bits_to_float_converter::do_task() {
  char offset;
  int bytes_read = 0;

  do {
    bytes_read = data_reader->get_bytes(1, &offset);
  } while (bytes_read == 0);
  
  assert(bytes_read == 1);
  assert((offset >= 0) && (offset*bits_per_sample < 8));

  // produce size_output_slice number of samples
  assert(has_work());

  assert(bits_per_sample > 0);
  assert(size_output_slice > 0);
  assert(size_output_slice % (8/bits_per_sample) == 0);
  Output_buffer_element &buffer = output_buffer->produce();

  if (buffer.size() < size_output_slice) {
    buffer.resize(size_output_slice);
  }

  assert((int)input_char_buffer.size() ==
         (size_output_slice/(8/bits_per_sample)+1));


  // Add one byte for the offset
  int bytes_to_read = input_char_buffer.size();
  bytes_read = 0;
  do {
    bytes_read +=
      data_reader->get_bytes(bytes_to_read-bytes_read,
                             &input_char_buffer[bytes_read]);
  } while (bytes_read != bytes_to_read);

  if (bits_per_sample == 2) {
    FLOAT *output_buffer = &buffer[0];
    // First byte:
    memcpy(output_buffer,
           &lookup_table[(int)(unsigned char)input_char_buffer[0]][(int)offset],
           (4-offset)*sizeof(FLOAT));
    output_buffer += 4-offset;

    for (int byte = 1; byte < bytes_read-1; byte++) {
      memcpy(output_buffer, // byte * 4
             &lookup_table[(int)(unsigned char)input_char_buffer[byte]][0],
             4*sizeof(FLOAT));
      output_buffer += 4;
    }

    // Last byte:
    memcpy(output_buffer,
           &lookup_table[(int)(unsigned char)input_char_buffer[bytes_read-1]][0],
           offset*sizeof(FLOAT));
  } else {
    std::cout << "Not yet implemented" << std::endl;
    assert(false);
  }

  output_buffer->produced(size_output_slice);
}

bool
Bits_to_float_converter::has_work() {
  if (bits_per_sample <= 0)
    return false;
  if (data_reader->get_size_dataslice() <= 0)
    return false;
  if (output_buffer->full())
    return false;
  return true;
}
