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
  void send_release(Input_memory_pool_element &elem);

  void do_bit_offset(std::vector<unsigned char> &data,
                     unsigned char last_sample,
                     int offset);

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

  Output_memory_pool   memory_pool;

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
    memory_pool(10)
    /**/
{
  assert(!memory_pool.empty());
}

template <class Type>
void
Integer_delay_correction_per_channel<Type>::do_task() {
  assert(has_work());
  assert(current_delay.first <= 0);

  Input_buffer_element input_element = input_buffer_->front();
  int byte_offset =
    (_current_time-input_element.start_time)*sample_rate*bits_per_sample/8/1000000 +
    current_delay.first;
  if (byte_offset >= (int)input_element.channel_data.data().data.size()) {
    // This can happen when we go to a next integration slice
    input_buffer_->front().channel_data.release();
    input_buffer_->pop();
    input_element = input_buffer_->front();
    DEBUG_MSG_RANK(8, "New block, time: " << input_element.start_time);
    return;
  }

  Output_buffer_element output_element;
  output_element.fft_data = memory_pool.allocate();
  if (output_element.fft_data.data().data.size() != (size_t)nr_output_bytes) {
    output_element.fft_data.data().data.resize(nr_output_bytes);
  }

  if (byte_offset < -nr_output_bytes) {
    // Completely random data
    for (int i=0; i<nr_output_bytes; i++) {
      output_element.fft_data.data().data[i] = 0;
    }
    // Do not do the bit offset
  } else if (byte_offset < 0) {
    // Partially random data
    for (int i=0; i<-byte_offset; i++) {
      output_element.fft_data.data().data[i] = 0;
    }
    for (int i=-byte_offset; i<nr_output_bytes; i++) {
      output_element.fft_data.data().data[i] =
        input_element.channel_data.data().data[i+byte_offset];
    }
    do_bit_offset(output_element.fft_data.data().data,
                  input_element.channel_data.data().data[nr_output_bytes+byte_offset],
                  current_delay.second);

  } else {
    assert (byte_offset >= 0);
    int input_data_size = input_element.channel_data.data().data.size();
    if ((byte_offset + nr_output_bytes + 1) < input_data_size) {
      for (int i=0; i<nr_output_bytes; i++) {
        output_element.fft_data.data().data[i] =
          input_element.channel_data.data().data[i+byte_offset];
      }
      assert(nr_output_bytes+byte_offset <
             (int)input_element.channel_data.data().data.size());
      do_bit_offset(output_element.fft_data.data().data,
                    input_element.channel_data.data().data[nr_output_bytes+byte_offset],
                    current_delay.second);
    } else {
      int bytes_in_current_block = input_data_size-byte_offset;
      for (int i=0; i<bytes_in_current_block; i++) {
        output_element.fft_data.data().data[i] =
          input_element.channel_data.data().data[i+byte_offset];
      }
      input_buffer_->front().channel_data.release();
      input_buffer_->pop();
      input_element = input_buffer_->front();
      assert(!input_buffer_->empty());
      input_element = input_buffer_->front();
      for (int i=bytes_in_current_block; i<nr_output_bytes; i++) {
        output_element.fft_data.data().data[i] =
          input_element.channel_data.data().data[i-bytes_in_current_block];
      }
      assert(nr_output_bytes-bytes_in_current_block <
             (int)input_element.channel_data.data().data.size());
      do_bit_offset(output_element.fft_data.data().data,
                    input_element.channel_data.data().data[nr_output_bytes-bytes_in_current_block],
                    current_delay.second);
    }

  }

  output_buffer_->push(output_element);

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
  if (memory_pool.number_free_element() < 2)
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
  int delay_in_bits = delay_in_samples*bits_per_sample-delay_in_bytes*8;

  if (delay_in_bits == 8) {
    delay_in_bytes++;
    delay_in_bits = 0;
  }

  assert((delay_in_bytes <= 0) && (delay_in_bits < 8));
  assert((delay_in_bytes*8 + delay_in_bits)/bits_per_sample == delay_in_samples);

  return Delay_type(delay_in_bytes, delay_in_bits);
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
  nr_output_bytes = parameters.number_channels*bits_per_sample/8;
  sample_rate = parameters.track_bit_rate * subsamples_per_sample;

  assert(((nr_output_bytes*(8/bits_per_sample))*1000000) % sample_rate== 0);
  delta_time = (nr_output_bytes*(8/bits_per_sample))*1000000/sample_rate;
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
void
Integer_delay_correction_per_channel<Type>::
do_bit_offset(std::vector<unsigned char> &data,
              unsigned char last_sample,
              int offset) {
  assert((int)data.size() == nr_output_bytes);
  // Bit offset
  if (offset != 0) {
    assert(offset % 2 == 0);
    const int offset_rev = 8-offset;
    for (int i=0; i<nr_output_bytes-1; i++) {
      data[i] =
        ((unsigned char)(data[i] << offset)) |
        ((unsigned char)(data[i+1] >> offset_rev));
    }
    // And process the last byte:
    data[nr_output_bytes-1] =
      ((unsigned char)(data[nr_output_bytes-1] << offset)) |
      ((unsigned char)(last_sample >> offset_rev));
  }
}

#endif // INTEGER_DELAY_CORRECTION_PER_CHANNEL_H
