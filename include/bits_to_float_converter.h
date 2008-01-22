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

#include <boost/shared_ptr.hpp>

#include "channel_extractor.h"
#include "data_reader.h"
#include "semaphore_buffer.h"
#include "tasklet/tasklet.h"
#include <fstream>

class Bits_to_float_converter : public Tasklet
{
public:
  typedef Buffer_element_vector<FLOAT>             Output_buffer_element;
  typedef Semaphore_buffer< Output_buffer_element > Output_buffer;
  typedef boost::shared_ptr<Output_buffer>          Output_buffer_ptr;
  typedef boost::shared_ptr<Data_reader>            Data_reader_ptr;

  Bits_to_float_converter();
  
  /// Set the number of bits per data sample
  void set_parameters(int nbits_per_sample, 
                      int size_input_slice, 
                      int size_output_slice);

  const char *name() { return __PRETTY_FUNCTION__; }
  void do_task();
  bool has_work();

  bool all_data_read() {
    return (data_reader->get_size_dataslice() <= 0);
  }
  
  /** Sets the input for the Bits_to_float_converter to a data reader.
   * This assumes that the samples are encoded in bytes
   **/
  void set_data_reader(boost::shared_ptr<Data_reader> data_reader);
  
  Output_buffer_ptr get_output_buffer();

private:
  int bits_per_sample;
  int size_output_slice;
  
  Output_buffer_ptr  output_buffer;

  Data_reader_ptr                      data_reader;
  // Buffer for the data samples stored in characters
  std::vector<char>                    intermediate_buffer;
};

#endif /* BITS_TO_FLOAT_CONVERTER_H_ */
