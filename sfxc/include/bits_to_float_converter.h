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

#include "data_reader.h"
#include "semaphore_buffer.h"
#include "tasklet/tasklet.h"

#include "correlator_node_types.h"

#include <fstream>

class Bits_to_float_converter : public Tasklet
{
public:
  typedef Correlator_node_types::Bit_sample_buffer     Input_buffer;
  typedef Correlator_node_types::Bit_sample_buffer_ptr Input_buffer_ptr;
  typedef Input_buffer::value_type                     Input_buffer_element;

  typedef Correlator_node_types::Fourier_memory_pool_element Output_buffer_element;
  typedef Correlator_node_types::Fourier_memory_pool   Output_memory_pool;
  typedef Correlator_node_types::Fourier_buffer        Output_buffer;
  typedef Correlator_node_types::Fourier_buffer_ptr    Output_buffer_ptr;

  Bits_to_float_converter();
  
  /// Set the number of bits per data sample
  void set_parameters(int nbits_per_sample);

  const char *name() { return __PRETTY_FUNCTION__; }
  void do_task();
  bool has_work();
  
  /** Sets the input for the Bits_to_float_converter to a data reader.
   * This assumes that the samples are encoded in bytes
   **/
  void connect_to(Input_buffer_ptr buffer);
  
  Output_buffer_ptr get_output_buffer();

private:
  int                bits_per_sample;
  
  Input_buffer_ptr    input_buffer;

  Output_memory_pool  output_memory_pool;
  Output_buffer_ptr   output_buffer;
  
  // only 2 bit samples
  FLOAT lookup_table[256][4];
};

#endif /* BITS_TO_FLOAT_CONVERTER_H_ */
