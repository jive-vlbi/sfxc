/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Channel_extractor_mark4.h 219 2007-05-09 11:55:38Z kruithof $
 *
 */

#ifndef BITS_TO_FLOAT_CONVERTER_H_
#define BITS_TO_FLOAT_CONVERTER_H_

#include <Channel_extractor.h>
#include <Data_reader.h>
#include <boost/shared_ptr.hpp>

class Bits_to_float_converter
{
public:
	Bits_to_float_converter();
  
  /** Read a number of samples from input **/
  size_t get_data(size_t nSamples, double *buffer); 

  /// Set the number of bits per data sample
  void set_bits_per_sample(int nbits);
  
  /** Sets the input for the Bits_to_float_converter to a data reader.
   * This assumes that the samples are encoded in bytes
   **/
  void set_data_reader(boost::shared_ptr<Data_reader> data_reader);
  
  uint64_t data_counter();
  int get_size_dataslice();

private:
  int bits_per_sample;

  boost::shared_ptr<Data_reader>       data_reader;
};

#endif /* BITS_TO_FLOAT_CONVERTER_H_ */
