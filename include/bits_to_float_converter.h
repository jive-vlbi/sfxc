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
  typedef Correlator_node_types::Bit_sample_queue     Input_buffer;
  typedef Correlator_node_types::Bit_sample_queue_ptr Input_buffer_ptr;
  typedef Input_buffer::value_type                    Input_buffer_element;

  typedef Buffer_element_vector<FLOAT>                Output_buffer_element;
  typedef Semaphore_buffer< Output_buffer_element >   Output_buffer;
  typedef boost::shared_ptr<Output_buffer>            Output_buffer_ptr;


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
  
  Output_buffer_ptr  output_buffer;

  Input_buffer_ptr   input_buffer;
  
  // only 2 bit samples
  FLOAT lookup_table[256][4];
};

#endif /* BITS_TO_FLOAT_CONVERTER_H_ */
