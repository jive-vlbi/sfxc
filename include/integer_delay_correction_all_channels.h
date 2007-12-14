/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef INTEGER_DELAY_CORRECTION_ALL_CHANNELS_H
#define INTEGER_DELAY_CORRECTION_ALL_CHANNELS_H

#include "semaphore_buffer.h"
#include "input_node_types.h"

template <class Type>
class Integer_delay_correction_all_channels {
public:
  typedef typename Input_node_types<Type>::Mk4_buffer    Input_buffer;
  typedef typename Input_buffer::value_type              Input_buffer_element;
  typedef boost::shared_ptr<Input_buffer>                Input_buffer_ptr;

  typedef typename Input_node_types<Type>::Fft_buffer    Output_buffer;
  typedef typename Output_buffer::value_type             Output_buffer_element;
  typedef boost::shared_ptr<Output_buffer>               Output_buffer_ptr;
  
  Integer_delay_correction_all_channels();
  
  /// For tasklet
  
  /// Process one piece of data
  void do_task();
  /// Check if we can process data
  bool has_work();
  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);
  /// Get the output
  Output_buffer_ptr get_output_buffer();
  
private:
  
private:
  Input_buffer_ptr input_buffer_;
  Output_buffer_ptr output_buffer_;
};

template <class Buffer>
Integer_delay_correction_all_channels<Buffer>::Integer_delay_correction_all_channels() {
  output_buffer_ = Output_buffer_ptr(new Output_buffer(10));
}

template <class Buffer>
void
Integer_delay_correction_all_channels<Buffer>::do_task() {
  assert(has_work());
  int size;
  Input_buffer_element &input_element = input_buffer_->consume(size);
  Output_buffer_element &output_element = output_buffer_->produce();
  
  output_element.data = input_element;
  output_element.start_index = 0;
  
  input_buffer_->consumed();
  output_buffer_->produced(size);
}

template <class Buffer>
bool
Integer_delay_correction_all_channels<Buffer>::has_work() {
  assert(output_buffer_ != Output_buffer_ptr());
  if (input_buffer_ == Input_buffer_ptr()) return false;
  
  if (input_buffer_->empty()) return false;
  if (output_buffer_->full()) return false;
  return true;
}

template <class Buffer>
void 
Integer_delay_correction_all_channels<Buffer>::
connect_to(Input_buffer_ptr buffer) {
  input_buffer_ = buffer;
}

template <class Buffer>
typename Integer_delay_correction_all_channels<Buffer>::Output_buffer_ptr 
Integer_delay_correction_all_channels<Buffer>::
get_output_buffer() {
  return output_buffer_;
}

#endif // INTEGER_DELAY_CORRECTION_ALL_CHANNELS_H
