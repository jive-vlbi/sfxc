/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef INTEGER_DELAY_CORRECTION_PER_CHANNEL_H
#define INTEGER_DELAY_CORRECTION_PER_CHANNEL_H

#include "semaphore_buffer.h"
#include "input_node_types.h"
#include "utils.h"

template <class Type>
class Integer_delay_correction_per_channel {
public:
  typedef typename Input_node_types<Type>::Channel_buffer     Input_buffer;
  typedef typename Input_node_types<Type>::Channel_buffer_element Input_buffer_element;
  typedef typename Input_node_types<Type>::Channel_buffer_ptr Input_buffer_ptr;

  typedef typename Input_node_types<Type>::Channel_memory_pool
  /**/                                                        Input_memory_pool;
  typedef typename Input_node_types<Type>::Channel_memory_pool_element
  /**/                                                        Input_memory_pool_element;

  typedef typename Input_node_types<Type>::Channel_memory_pool    Output_memory_pool;
  typedef typename Input_node_types<Type>::Fft_buffer_element Output_buffer_element;
  typedef typename Input_node_types<Type>::Fft_buffer         Output_buffer;
  typedef typename Input_node_types<Type>::Fft_buffer_ptr     Output_buffer_ptr;
  typedef typename Input_node_types<Type>::Channel_memory_pool_element Input_data_block;

  // Pair of the delay in samples (of type Type) and subsamples
  typedef std::pair<int,int>                                      Delay_type;

  Integer_delay_correction_per_channel();
  ~Integer_delay_correction_per_channel() {}

  /// For tasklet

  /// Process one piece of data
  void do_task();
  /// Check if we can process data
  bool has_work();
  /// Set the input
  void connect_to(Input_buffer_ptr new_Input_buffer);
  /// Get the output
  const char *name() {
    return "Integer_delay_correction_per_channel";
  }

  Output_buffer_ptr get_output_buffer();

  // Set the delay table
  bool time_set() {
    return _current_time > 0;
  }
  void set_time(int64_t time);
  void set_stop_time(int64_t time);
  void set_delay_table(Delay_table_akima &table);
  void set_parameters(const Input_node_parameters &parameters,
                      int node_nr);

  int64_t current_time() {
    return _current_time;
  }
  int bytes_of_output(int nr_seconds);

  double delay(int64_t time);

private:
  Delay_type get_delay(int64_t time);

  Input_data_block allocate_random_element();

private:
  /// The input data Type
  Input_buffer_ptr     input_buffer_;
  /// The output data Type
  Output_buffer_ptr    output_buffer_;
  /// Number of samples to output (number_channels/subsamples_per_sample)
  int                  nr_output_bytes;
  /// Number of bits per subsample (1 or 2)
  int                  bits_per_sample;
  /// Data rate (of samples of type Type) in Hz
  int                  sample_rate;
  /// Current time in microseconds
  int64_t              _current_time;
  /// Stop time in microseconds
  int64_t              stop_time_;
  /// Length of one output data block in microseconds
  int64_t              delta_time;

  /// Number of bytes per integration_slice per channel
  int nr_bytes_per_integration_slice;

  /// Integration time in microseconds
  int                  integration_time;
  /// Delay for the sample to process
  Delay_type           current_delay;

  // Memory pool for random data
  Output_memory_pool   memory_pool_;

  /// Delay table
  Delay_table_akima    delay_table;
};

template <class Type>
Integer_delay_correction_per_channel<Type>::
Integer_delay_correction_per_channel()
    : output_buffer_(new Output_buffer()),
    nr_output_bytes(-1),
    bits_per_sample(-1),
    sample_rate(-1),
    _current_time(-1),
    stop_time_(-1),
    integration_time(-1),
    current_delay(1,1),
    memory_pool_(10)
    /**/
{
  assert(!memory_pool_.empty());
}

template <class Type>
void
Integer_delay_correction_per_channel<Type>::do_task() {
  assert(has_work());
  assert(current_delay.first <= 0);

  Input_buffer_element input_element = input_buffer_->front();
  Output_buffer_element output_element;
  output_element.delay = (char)current_delay.second;
  output_element.release_data = false;

  int byte_offset =
    (_current_time-input_element.start_time)*sample_rate*bits_per_sample/8/1000000 +
    current_delay.first;
  if (byte_offset >= (int)input_element.channel_data.data().data.size()) {
    // This can happen when we go to a next integration slice
    output_element.channel_data = input_element.channel_data;
    output_element.release_data = true;
    output_buffer_->push(output_element);

    input_buffer_->pop();
    return;
  }

  if (byte_offset < -nr_output_bytes) {
    // Completely random data

    // Do not do the bit offset
    output_element.delay = char(0);

    // Send random data
    output_element.channel_data = allocate_random_element();
    output_element.first_sample = 0;
    output_element.nr_samples = nr_output_bytes;
    output_buffer_->push(output_element);

    // Release the data
    output_element.release_data = true;
    output_buffer_->push(output_element);

  } else if (byte_offset < 0) {
    // Partially random data

    // Send random data
    output_element.channel_data = allocate_random_element();
    output_element.first_sample = 0;
    output_element.nr_samples = -byte_offset;
    output_buffer_->push(output_element);

    // Release the data
    output_element.release_data = true;
    output_buffer_->push(output_element);

    // Send real data
    output_element.release_data = false;
    // Don't send the delay again:
    output_element.delay = -1;
    output_element.channel_data = input_element.channel_data;
    output_element.first_sample = 0;
    output_element.nr_samples = nr_output_bytes+byte_offset;
    output_buffer_->push(output_element);

  } else {
    assert (byte_offset >= 0);
    int input_data_size = input_element.channel_data.data().data.size();
    if ((byte_offset + nr_output_bytes) < input_data_size) {
      // Send data
      output_element.channel_data = input_element.channel_data;
      output_element.first_sample = byte_offset;
      output_element.nr_samples = nr_output_bytes;
      output_buffer_->push(output_element);

    } else {
      int bytes_in_current_block = input_data_size-byte_offset;
      // Send first block of data
      output_element.channel_data = input_element.channel_data;
      output_element.first_sample = byte_offset;
      output_element.nr_samples = bytes_in_current_block;
      output_buffer_->push(output_element);

      // Release the data
      output_element.release_data = true;
      output_buffer_->push(output_element);

      // Get the second block of data
      input_buffer_->pop();
      assert(!input_buffer_->empty());
      input_element = input_buffer_->front();

      // Send second block of data
      output_element.release_data = false;
      // Don't send the delay again:
      output_element.delay = -1;
      output_element.channel_data = input_element.channel_data;
      output_element.first_sample = 0;
      output_element.nr_samples = nr_output_bytes-bytes_in_current_block;
      output_buffer_->push(output_element);
    }
  }

  _current_time += delta_time;
  current_delay = get_delay(_current_time);

  if ((_current_time/integration_time) !=
      ((_current_time+delta_time-1)/integration_time)) {
    // Continue with the next integration slice
    _current_time =
      ((_current_time+delta_time-1)/integration_time)*integration_time;
    current_delay = get_delay(_current_time);
  }
}

template <class Type>
bool
Integer_delay_correction_per_channel<Type>::has_work() {
  assert(output_buffer_ != Output_buffer_ptr());
  if ((stop_time_ > 0) && (_current_time >= stop_time_))
    return false;
  if (sample_rate <= 0)
    return false;
  if (current_delay.first > 0)
    return false;
  if (input_buffer_ == Input_buffer_ptr())
    return false;
  if (input_buffer_->empty())
    return false;
  if (memory_pool_.empty())
    return false;

  // Check whether we cross a block boundary:
  Input_buffer_element &input_element = input_buffer_->front();
  int byte_offset =
    (_current_time-input_element.start_time)*sample_rate*bits_per_sample/8/1000000 +
    current_delay.first;
  if (size_t(byte_offset + nr_output_bytes +1) >=
      input_element.channel_data.data().data.size()) {
    if (input_buffer_->size() < 2)
      return false;
  }

  return true;
}

template <class Type>
void
Integer_delay_correction_per_channel<Type>::
connect_to(Input_buffer_ptr buffer) {
  input_buffer_ = buffer;
}

template <class Type>
typename Integer_delay_correction_per_channel<Type>::Output_buffer_ptr
Integer_delay_correction_per_channel<Type>::
get_output_buffer() {
  return output_buffer_;
}

template <class Type>
double
Integer_delay_correction_per_channel<Type>::delay(int64_t time) {
  return delay_table.delay(time+delta_time/2);
}


template <class Type>
typename Integer_delay_correction_per_channel<Type>::Delay_type
Integer_delay_correction_per_channel<Type>::get_delay(int64_t time) {
  assert(delay_table.initialised());
  assert(delta_time > 0);
  assert(delta_time%2 == 0);
  double delay_ = delay(time);
  int delay_in_samples = (int)std::floor(delay_*sample_rate+.5);

  // All because modulo doesn't work for negative values
  assert(delay_in_samples < 0);

  int delay_in_bytes = -((-delay_in_samples)/(8/bits_per_sample))-1;
  int delay_in_remaining_samples = delay_in_samples-delay_in_bytes*8/bits_per_sample;

  if (delay_in_remaining_samples*bits_per_sample == 8) {
    delay_in_bytes++;
    delay_in_remaining_samples = 0;
  }

  assert((delay_in_bytes <= 0) && (delay_in_remaining_samples < 8));
  assert((delay_in_bytes*8 + delay_in_remaining_samples*bits_per_sample)/bits_per_sample == delay_in_samples);

  if (delay_in_remaining_samples >= 4) {
    DEBUG_MSG("delay: " << delay_in_remaining_samples);
  }
  return Delay_type(delay_in_bytes, delay_in_remaining_samples);
}

template <class Type>
void
Integer_delay_correction_per_channel<Type>::
set_delay_table(Delay_table_akima &table) {
  delay_table = table;
}

template <class Type>
void
Integer_delay_correction_per_channel<Type>::
set_parameters(const Input_node_parameters &parameters,
               int node_nr) {
  bits_per_sample = parameters.bits_per_sample();
  int subsamples_per_sample = parameters.subsamples_per_sample();
  assert(parameters.number_channels%subsamples_per_sample == 0);

  assert((parameters.number_channels*bits_per_sample)%8 == 0);
  // The offset is not counted
  nr_output_bytes = parameters.number_channels*bits_per_sample/8+1;
  sample_rate = parameters.track_bit_rate * subsamples_per_sample;

  assert((((nr_output_bytes-1)*(8/bits_per_sample))*1000000) % sample_rate== 0);
  delta_time = ((nr_output_bytes-1)*(8/bits_per_sample))*1000000/sample_rate;
  integration_time = parameters.integr_time*1000;

  nr_bytes_per_integration_slice =
    Control_parameters::nr_bytes_per_integration_slice_input_node_to_correlator_node
    (parameters.integr_time,
     sample_rate,
     bits_per_sample,
     parameters.number_channels);


}

template <class Type>
void
Integer_delay_correction_per_channel<Type>::
set_time(int64_t time) {
  assert(delay_table.initialised());
  _current_time = time;

  current_delay = get_delay(_current_time);
}

template <class Type>
void
Integer_delay_correction_per_channel<Type>::
set_stop_time(int64_t time) {
  stop_time_ = time;
}

template <class Type>
int
Integer_delay_correction_per_channel<Type>::
bytes_of_output(int nr_seconds) {
  if (nr_seconds < 0)
    return nr_seconds;

  return nr_bytes_per_integration_slice;
}


template <class Type>
typename
Integer_delay_correction_per_channel<Type>::Input_data_block
Integer_delay_correction_per_channel<Type>::
allocate_random_element() {
  Input_data_block result = memory_pool_.allocate();
  if (result.data().data.size() != (size_t)nr_output_bytes) {
    result.data().data.resize(nr_output_bytes);
  }
  // Completely random data
  for (int i=0; i<nr_output_bytes; i++) {
#ifdef SFXC_DETERMINISTIC
    // Set data to zero
    result.data().data[i] = 0;
#else
    // Randomize data
    result.data().data[i] = (char)park_miller_random();
#endif

  }
  return result;
}
#endif // INTEGER_DELAY_CORRECTION_PER_CHANNEL_H
