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
  : bits_per_sample(-1),
    output_memory_pool(10),
    output_buffer(new Output_buffer()) {
  for (int i=0; i<256; i++) {
    lookup_table[i][0] = sample_value_ms[(i>>6) & 3];
    lookup_table[i][1] = sample_value_ms[(i>>4) & 3];
    lookup_table[i][2] = sample_value_ms[(i>>2) & 3];
    lookup_table[i][3] = sample_value_ms[i & 3];
  }
}

void
Bits_to_float_converter::set_parameters(int nbits_per_sample_) {
  bits_per_sample = nbits_per_sample_;

  assert(bits_per_sample == 2);
}

void
Bits_to_float_converter::
connect_to(Input_buffer_ptr buffer) {
  assert(buffer != Input_buffer_ptr());
  input_buffer = buffer;
}

Bits_to_float_converter::Output_buffer_ptr
Bits_to_float_converter::get_output_buffer() {
  assert(output_buffer != Output_buffer_ptr());
  return output_buffer;
}

void Bits_to_float_converter::do_task() {
  // produce size_output_slice number of samples
  assert(has_work());

  assert(!input_buffer->empty());
  Input_buffer_element &input_elem = input_buffer->front();

  char offset = input_elem->offset;
  
  assert(bits_per_sample > 0);
  int size_output_slice = 8/bits_per_sample*(input_elem->data.size()-1);

  assert(size_output_slice > 0);
  assert(size_output_slice % (8/bits_per_sample) == 0);
  assert(offset >= 0);
  assert(offset < (8/bits_per_sample));
  assert(output_buffer != Output_buffer_ptr());
  assert(!output_memory_pool.empty());
  Output_buffer_element output_elem = output_memory_pool.allocate();

  // Check for the right size is done in the resize method
  // A factor of 2 for padding in the correlator
  output_elem->resize(2*size_output_slice);

  if (bits_per_sample == 2) {
    FLOAT *output_buffer = output_elem->buffer();
    // First byte:
    memcpy(output_buffer,
           &lookup_table[(int)input_elem->data[0]][(int)offset],
           (4-offset)*sizeof(FLOAT));
    output_buffer += 4-offset;

    int input_size = input_elem->data.size();
    for (int byte = 1; byte < input_size-1; byte++) {
      memcpy(output_buffer, // byte * 4
             &lookup_table[(int)input_elem->data[byte]][0],
             4*sizeof(FLOAT));
      output_buffer += 4;
    }

    // Last byte:
    memcpy(output_buffer,
           &lookup_table[(int)(unsigned char)input_elem->data[input_size-1]][0],
           offset*sizeof(FLOAT));
    output_buffer += offset;

    for (int i=size_output_slice; i<2*size_output_slice; i++) {
      output_buffer = 0; output_buffer++;
    }
  } else {
    std::cout << "Not yet implemented" << std::endl;
    assert(false);
  }

  input_elem.release();
  input_buffer->pop();
  output_buffer->push(output_elem);
}

bool
Bits_to_float_converter::has_work() {
  if (bits_per_sample <= 0)
    return false;
  if (input_buffer->empty())
    return false;
  if (output_memory_pool.empty())
    return false;
  return true;
}
