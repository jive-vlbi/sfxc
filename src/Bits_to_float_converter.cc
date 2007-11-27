/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Channel_extractor_mark4.h 219 2007-05-09 11:55:38Z kruithof $
 *
 */

#include <Bits_to_float_converter.h>
#include <assert.h>

const double sample_value_ms[] = {-7, 2, -2, 7};
const double sample_value_m[]  = {-5,  5};

Bits_to_float_converter::Bits_to_float_converter()
 : bits_per_sample(-1), size_output_slice(-1),
   output_buffer(Output_buffer_ptr(new Output_buffer(10)))
{
}

void 
Bits_to_float_converter::set_parameters(int bits_per_sample_,
                                        int size_input_slice_,
                                        int size_output_slice_) {
  bits_per_sample = bits_per_sample_;
  size_output_slice = size_output_slice_;
  intermediate_buffer.resize(size_output_slice/(8/bits_per_sample));

  assert(data_reader != Data_reader_ptr());

  // NGHK: TODO BugFix, not a real fix
  if (data_reader->get_size_dataslice() > 0) {
    data_reader->get_bytes(data_reader->get_size_dataslice(), NULL);
  }

  assert(data_reader->get_size_dataslice() <= 0);
  data_reader->set_size_dataslice(size_input_slice_);
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
  if (!output_buffer->full()) {
    assert(bits_per_sample > 0);
    assert(size_output_slice > 0);
    assert(size_output_slice % (8/bits_per_sample) == 0);
    Output_buffer_element &buffer = output_buffer->produce();
    
    if (buffer.size() < size_output_slice) {
      buffer.resize(size_output_slice);
    }
    
    assert((int)intermediate_buffer.size() == 
           size_output_slice/(8/bits_per_sample));
    
    if (bits_per_sample == 2) {
      size_t bytes_read = 0;
      while (bytes_read != size_output_slice/4) {
        bytes_read += data_reader->get_bytes(size_output_slice/4-bytes_read, 
                                             &intermediate_buffer[bytes_read]);
      }
      
      assert(bytes_read == size_output_slice/4);

      int sample = 0;    
      for (size_t byte = 0; byte < bytes_read; byte++) {
        buffer[sample] = sample_value_ms[ intermediate_buffer[byte]&3 ];
        sample++; 
        buffer[sample] = sample_value_ms[ (intermediate_buffer[byte]>>2)&3 ];
        sample++; 
        buffer[sample] = sample_value_ms[ (intermediate_buffer[byte]>>4)&3 ];
        sample++; 
        buffer[sample] = sample_value_ms[ (intermediate_buffer[byte]>>6)&3 ];
        sample++; 
      }

      assert(sample == size_output_slice);
    } else {
      std::cout << "Not yet implemented" << std::endl;
      assert(false);
    }

    output_buffer->produced(size_output_slice);
  }
}
