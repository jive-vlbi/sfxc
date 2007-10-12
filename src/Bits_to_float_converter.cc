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
 : bits_per_sample(0)
{
  
}


size_t 
Bits_to_float_converter::get_data(size_t nSamples, double *buffer) {
  assert(bits_per_sample > 0);
  assert(data_reader != NULL);
  if (bits_per_sample == 2) {
    char bit_samples[nSamples/4];
    size_t bytes_read = data_reader->get_bytes(nSamples/4, bit_samples);

    int sample = 0;    
    for (size_t byte = 0; byte < bytes_read; byte++) {
      buffer[sample] = sample_value_ms[ bit_samples[byte]&3 ];
      sample++; 
      buffer[sample] = sample_value_ms[ (bit_samples[byte]>>2)&3 ];
      sample++; 
      buffer[sample] = sample_value_ms[ (bit_samples[byte]>>4)&3 ];
      sample++; 
      buffer[sample] = sample_value_ms[ (bit_samples[byte]>>6)&3 ];
      sample++; 
    }
    return sample;
  } else {
    std::cout << "Not yet implemented" << std::endl;
    assert(false);
  }
  return 0;
} 

void 
Bits_to_float_converter::set_bits_per_sample(int nbits) {
  bits_per_sample = nbits;
}

void 
Bits_to_float_converter::set_data_reader(boost::shared_ptr<Data_reader> data_reader_) {
  data_reader = data_reader_;
}
 
uint64_t 
Bits_to_float_converter::data_counter() {
  return (data_reader->data_counter()*8)/bits_per_sample;
}

int 
Bits_to_float_converter::get_size_dataslice() {
  return (data_reader->get_size_dataslice()*8)/bits_per_sample;
}
