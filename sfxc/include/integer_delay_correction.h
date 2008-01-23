/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef INTEGER_DELAY_CORRECTION_H
#define INTEGER_DELAY_CORRECTION_H

#include "tasklet/tasklet.h"
#include "bits_to_float_converter.h"
#include "semaphore_buffer.h"
#include "delay_table_akima.h"
#include "control_parameters.h"
#include "data_reader_buffer.h"

class Integer_delay_correction : public Tasklet
{
public:
  typedef Bits_to_float_converter::Output_buffer_element Input_buffer_element;
  typedef Bits_to_float_converter::Output_buffer         Input_buffer;
  typedef Bits_to_float_converter::Output_buffer_ptr     Input_buffer_ptr;

  typedef Buffer_element_vector<FLOAT>                  Output_buffer_element;
  typedef Semaphore_buffer<Output_buffer_element>        Output_buffer;
  typedef boost::shared_ptr<Output_buffer>               Output_buffer_ptr;
  
  typedef Data_reader_buffer<Input_buffer_element>       Input_reader;
  typedef boost::shared_ptr< Input_reader >              Input_reader_ptr;

  Integer_delay_correction();
  virtual ~Integer_delay_correction();
  
  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);
  
  /// Get the output
  Output_buffer_ptr get_output_buffer();
  
  /// Set the delay table
  void set_delay_table(const Delay_table_akima &delay_table);
  
  void set_parameters(const Correlation_parameters &parameters);

  /// Do one delay step
  void do_task();
  const char *name() { return __PRETTY_FUNCTION__; }
  
  bool has_work();
  
private:
  int get_delay_in_samples(int64_t time);
  int delta_time();
private:
  Input_buffer_ptr    input_buffer;
  Input_reader_ptr    input_reader;
  Output_buffer_ptr   output_buffer;

  int64_t                current_time; // In microseconds
  Correlation_parameters correlation_parameters;
  
  int current_delay; // Integer delay in samples
  
  bool                delay_table_set;
  Delay_table_akima   delay_table;
  
  // Using a buffer of one element, only 1 sample difference in the delay
  Input_buffer_element::value_type buffered_element;
  
public:
  bool verbose;
};

#endif // INTEGER_DELAY_CORRECTION_H
