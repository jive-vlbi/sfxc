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
  // Pair of the delay in samples (of type Type) and subsamples
  typedef std::pair<int,int>                             Delay_type;

  Integer_delay_correction_all_channels();

  /// For tasklet

  /// Process one piece of data
  void do_task();
  /// Check if we can process data
  bool has_work();
  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);
  /// Get the output
  const char *name() {
    return "Integer_delay_correction_all_channels";
  }

  Output_buffer_ptr get_output_buffer();

  // Set the delay table
  void set_time(int time);
  void set_delay_table(Delay_table_akima &table);
  void set_parameters(Input_node_parameters &parameters);
private:
  Delay_type get_delay(int64_t time);

private:
  /// The input data buffer
  Input_buffer_ptr     input_buffer_;
  /// The output data buffer
  Output_buffer_ptr    output_buffer_;
  /// Number of samples to output (number_channels/subsamples_per_sample)
  int                  nr_output_samples;
  /// Number of bits per subsample (1 or 2)
  int                  bits_per_subsample;
  /// Number of subsamples in one sample of type Type
  int                  subsamples_per_sample;
  /// Position in the current data block
  int                  position;
  /// Data rate (of samples of type Type) in Hz
  int                  sample_rate;
  /// Current time in microseconds
  int64_t              current_time;
  /// Length of one output data block in microseconds
  int64_t              delta_time;

  /// Integration time in microseconds
  int                  integration_time;
  /// Delay for the sample to process
  Delay_type           current_delay;

  /// Delay table
  Delay_table_akima    delay_table;
};

template <class Buffer>
Integer_delay_correction_all_channels<Buffer>::Integer_delay_correction_all_channels()
    : output_buffer_(new Output_buffer()),
    nr_output_samples(-1),
    bits_per_subsample(-1),
    subsamples_per_sample(-1),
    position(-1),
    sample_rate(-1),
    current_time(-1),
    integration_time(-1),
    current_delay(0,0)
    /**/
{
  position = 0;
}

template <class Buffer>
void
Integer_delay_correction_all_channels<Buffer>::do_task() {
  assert(has_work());

  // get front input element
  Input_buffer_element &input_element = input_buffer_->front();

  // Check for the next integration slice, and skip partial fft-sizes then
//  if (current_time/integration_time !=
//      (current_time+delta_time)/integration_time) {
//    int64_t start_new_slice = 
//      ((current_time+delta_time)/integration_time)*integration_time;
//    int samples_to_read = 
//      (start_new_slice-current_time)*sample_rate/1000000-current_delay.first;
//    position += samples_to_read;
//    while (position >= (int)input_element.data().size()) {
//      position -= input_element.data().size();
//      input_buffer_->pop();
//    }
//  }

  // fill the output element
  Output_buffer_element output_element;
  output_element.data1               = input_element;
  output_element.number_data_samples = nr_output_samples;
  output_element.subsample_offset    = current_delay.second*bits_per_subsample;

  if (position < 0) {
    // Before actual data (delay at the beginning of the stream)
    if (nr_output_samples > -position) {
      // we can output actual data
      // Just use data1 twice: we will use weights later on anyway to randomize the data
      output_element.sample_offset = input_element.data().size()+position;
      output_element.data2 = input_element;
      output_element.last_sample = input_element.data()[nr_output_samples+1+position];
    } else {
      // Complete buffer of random data, just copy the current buffer
      output_element.sample_offset = 0;
      output_element.last_sample = 0;
    }
  } else {
    // normal case where we have data
    output_element.sample_offset = position;

    int samples_in_data2 = position+nr_output_samples+1-input_element.data().size();
    if (samples_in_data2 <= 0) {
      // Data is contained in one data block
      output_element.last_sample = input_element.data()[nr_output_samples+position];
    } else {
      // Get new data block
      input_buffer_->pop();
      output_element.data2 = input_buffer_->front();

      output_element.last_sample = output_element.data2.data()[samples_in_data2];

      if (samples_in_data2 == 1) {
        if (current_time/integration_time ==
            (current_time+delta_time)/integration_time) {
          Delay_type new_delay = get_delay(current_time+delta_time);
          if (new_delay.first < current_delay.first) {
            // Border case where the two blocks are used twice because of a delay change

            // Assumption: Delay changes one subsample at most
            assert(current_delay.second == 0);

            output_buffer_->push(output_element);

            // set time for the next block, it reuses the last sample of data1
            current_time += delta_time;
            position += nr_output_samples - 1;
            current_delay = new_delay;

            // reuse the last sample
            assert(position == (int)output_element.data1.data().size()-1);
            output_element.sample_offset = position;
            output_element.subsample_offset = current_delay.second;
            output_element.last_sample = output_element.data2.data()[nr_output_samples];
          }
        }
      }
    }
  }

  // Increase the position
  current_time += delta_time;
  position += nr_output_samples - current_delay.first;
  current_delay = get_delay(current_time);
  position += current_delay.first;

  // Check to avoid the case where position is negative
  if (position >= (int)input_element.data().size()) {
    position -= input_element.data().size();
    assert(position <= (int)output_element.data2.data().size());
    // NGHK: TODO: Check whether we already need to release the buffer
    output_element.release_data1 = true;
  }

  // Push the output to the buffer for further processing
  output_buffer_->push(output_element);
}

template <class Buffer>
bool
Integer_delay_correction_all_channels<Buffer>::has_work() {
  assert(output_buffer_ != Output_buffer_ptr());
  if (input_buffer_ == Input_buffer_ptr())
    return false;

  if (input_buffer_->size() < 2)
    return false;

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

template <class Buffer>
typename Integer_delay_correction_all_channels<Buffer>::Delay_type
Integer_delay_correction_all_channels<Buffer>::get_delay(int64_t time) {
  return Delay_type(0,0);
  double delay = delay_table.delay(time);
  double delay_in_samples = (delay*sample_rate)/1000;
  int sample_delay = (int)std::floor(delay_in_samples);
  int subsample_delay =
    (int)std::floor((delay_in_samples-sample_delay)*subsamples_per_sample);

  return Delay_type(sample_delay, subsample_delay);
}

template <class Buffer>
void
Integer_delay_correction_all_channels<Buffer>::
set_delay_table(Delay_table_akima &table) {
  delay_table = table;
}

template <class Buffer>
void
Integer_delay_correction_all_channels<Buffer>::
set_parameters(Input_node_parameters &parameters) {
  bits_per_subsample = parameters.bits_per_sample();
  subsamples_per_sample = parameters.subsamples_per_sample();
  assert(parameters.number_channels%subsamples_per_sample == 0);
  nr_output_samples = parameters.number_channels/subsamples_per_sample;
  sample_rate = parameters.track_bit_rate;
  delta_time = nr_output_samples*1000000/sample_rate;
  integration_time = parameters.integr_time*1000;
}

template <class Buffer>
void
Integer_delay_correction_all_channels<Buffer>::
set_time(int time) {
  current_time = int64_t(1000)*time;
  current_delay = get_delay(current_time);
  position = current_delay.first;
}

#endif // INTEGER_DELAY_CORRECTION_ALL_CHANNELS_H
