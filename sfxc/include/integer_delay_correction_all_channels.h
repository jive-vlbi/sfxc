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
#include "utils.h"

template <class Type>
class Integer_delay_correction_all_channels {
public:
  typedef typename Input_node_types<Type>::Mk4_buffer    Input_buffer;
  typedef typename Input_buffer::value_type              input_buffer_element;
  typedef boost::shared_ptr<Input_buffer>                input_buffer_ptr;

  typedef typename Input_node_types<Type>::Mk4_memory_pool
  /**/                                                   Input_memory_pool;
  typedef typename Input_node_types<Type>::Mk4_memory_pool_element
  /**/                                                   Input_memory_pool_element;

  typedef typename Input_node_types<Type>::Fft_buffer    Output_buffer;
  typedef typename Output_buffer::value_type             output_buffer_element;
  typedef boost::shared_ptr<Output_buffer>               output_buffer_ptr;
  // Pair of the delay in samples (of type Type) and subsamples
  typedef std::pair<int,int>                             Delay_type;

  Integer_delay_correction_all_channels();
  ~Integer_delay_correction_all_channels() {}

  /// For tasklet

  /// Process one piece of data
  void do_task();
  /// Check if we can process data
  bool has_work();
  /// Set the input
  void connect_to(input_buffer_ptr new_Input_buffer);
  /// Get the output
  const char *name() {
    return "Integer_delay_correction_all_channels";
  }

  output_buffer_ptr get_output_buffer();

  // Set the delay table
  bool time_set() {
    return current_time_ > 0;
  }
  void set_time(int64_t time);
  void set_stop_time(int64_t time);
  void set_delay_table(Delay_table_akima &table);
  void set_parameters(const Input_node_parameters &parameters,
                      int node_nr);

  int bytes_of_output(int nr_seconds);
private:
  Delay_type get_delay(int64_t time);
  void send_release(Input_memory_pool_element &elem);

  void pop_from_input_buffer();

private:
  /// The input data Type
  input_buffer_ptr     input_buffer_;
  /// The output data Type
  output_buffer_ptr    output_buffer_;
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
  int64_t              current_time_;
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

  /// Delay table
  Delay_table_akima    delay_table;

  /// Memory pool for a random data block as long as we don't have weights
  Input_memory_pool         memory_pool_;

  int node_nr_;
};

template <class Type>
Integer_delay_correction_all_channels<Type>::
Integer_delay_correction_all_channels()
    : output_buffer_(new Output_buffer()),
    nr_output_samples(-1),
    bits_per_subsample(-1),
    subsamples_per_sample(-1),
    position(-1),
    sample_rate(-1),
    current_time_(-1),
    stop_time_(-1),
    integration_time(-1),
    current_delay(1,1),
    memory_pool_(1)
    /**/
{
  position = 0;
  assert(!memory_pool_.empty());
}

template <class Type>
void
Integer_delay_correction_all_channels<Type>::do_task() {
  static int n_ffts=0;
  assert(has_work());
  assert(current_delay.first <= 0);

  bool release_data = false;
  output_buffer_element output_element;


  if (position >= (int)input_buffer_->front().data().size()) {
    position -= input_buffer_->front().data().size();

    output_element.data1 = input_buffer_->front();
    release_data = true;
  } else {

    // fill the output element
    output_element.data1               = input_buffer_->front();
    output_element.number_data_samples = nr_output_samples;
    output_element.subsample_offset    = current_delay.second*bits_per_subsample;

    if (position < 0) {
      Input_memory_pool_element random_element_ = memory_pool_.allocate();
      if (random_element_.data().size() != nr_output_samples+1) {
        random_element_.data().resize(nr_output_samples+1);
      }
      output_element.data1               = random_element_;

#ifdef SFXC_DETERMINISTIC
      { // Randomize data
        for (int i=0; i<nr_output_samples; i++) {
          // park_miller_random generates 31 random bits
          random_element_.data()[i] = Type(0);
        }
      }
#else
      { // Randomize data
        for (int i=0; i<nr_output_samples; i++) {
          // park_miller_random generates 31 random bits
          if (sizeof(Type) < 4) {
            random_element_.data()[i] = (Type)park_miller_random();
          } else if (sizeof(Type) == 4) {
            random_element_.data()[i] =
              ((Type(park_miller_random())<<16)&(~0xFFFF)) + 
              (park_miller_random()&0xFFFF);
          } else {
            assert(sizeof(Type) == 8);
            int64_t rnd = park_miller_random();
            rnd = (rnd << 16) + park_miller_random();
            rnd = (rnd << 16) + park_miller_random();
            rnd = (rnd << 16) + park_miller_random();
            random_element_.data()[i] = rnd;
          }
        }
      }
#endif
      release_data = true;
      
      // Before actual data (delay at the beginning of the stream)
      if (position < -nr_output_samples) {
        // Complete Type of random data, just copy the current Type
        output_element.sample_offset = 0;
      } else {
        // This fft contains partially valid data
        // we can output actual data
        // Just use data1 twice: we will use weights later on anyway to randomize the data
        output_element.sample_offset = output_element.data1.data().size()+position;
        output_element.data2 = input_buffer_->front();
      }
    } else {
      // normal case where we have data
      output_element.sample_offset = position;

      int samples_in_data2 = position+nr_output_samples+1-input_buffer_->front().data().size();
      if (samples_in_data2 < 0) {
        // Data is contained in one data block
      } else if (samples_in_data2 == 0) {
        // the samples come from the first block, but the extra sample from the next block

        // get the last sample
        position -= input_buffer_->front().data().size();
        pop_from_input_buffer();
        assert(!input_buffer_->empty());

        Delay_type new_delay = get_delay(current_time_+delta_time);
        if (new_delay.first < current_delay.first) {
          // Border case where the two blocks are used twice because of a delay change

          // Assumption: Delay changes one subsample at most
          assert(current_delay.second == 0);

          output_buffer_->push(output_element);
          n_ffts++;

          // set time for the next block, it reuses the last sample of data1
          current_time_ += delta_time;
          position += nr_output_samples - 1;
          current_delay = new_delay;

          // reuse the last sample
          assert(position == (int)output_element.data1.data().size()-1);
          output_element.data2 = input_buffer_->front();
          output_element.sample_offset = position;
          output_element.subsample_offset = current_delay.second;
        }

        // We can release the data
        release_data = true;
      } else {
        // Get new data block
        position -= input_buffer_->front().data().size();
        pop_from_input_buffer();
        assert(!input_buffer_->empty());
        output_element.data2 = input_buffer_->front();

        // We can release the data
        release_data = true;
      }
    }


    // Push the output to the Type for further processing
    output_buffer_->push(output_element);
    n_ffts++;

    if ((current_time_/integration_time) !=
        ((current_time_+delta_time)/integration_time)) {
      PROGRESS_MSG("node " << node_nr_
                << ", time " << current_time_+delta_time);
    }

    // Increase the position
    current_time_ += delta_time;
    position += nr_output_samples - current_delay.first;
    current_delay = get_delay(current_time_);
    position += current_delay.first;

    // Check for the next integration slice, and skip partial fft-sizes
    if ((current_time_/integration_time) !=
        ((current_time_+delta_time-1)/integration_time)) {

      position -= current_delay.first;

      int64_t start_new_slice =
        ((current_time_+delta_time)/integration_time)*integration_time;

      assert(start_new_slice > current_time_);

      int samples_to_read =
        (start_new_slice-current_time_)*sample_rate/1000000;

      position += samples_to_read;

      current_time_ = start_new_slice;
      current_delay = get_delay(current_time_);
      position += current_delay.first;
    }
  }
  if (release_data) {
    output_element.only_release_data1 = true;
    output_buffer_->push(output_element);
    return;
  }

}

template <class Type>
bool
Integer_delay_correction_all_channels<Type>::has_work() {
  assert(output_buffer_ != output_buffer_ptr());
  if ((stop_time_ > 0) && (current_time_ >= stop_time_))
    return false;
  if (sample_rate <= 0)
    return false;
  if (current_delay.first > 0)
    return false;
  if (input_buffer_ == input_buffer_ptr())
    return false;

  if (input_buffer_->empty())
    return false;
  if (position < 0) {
    // We need a block of random data
    if (memory_pool_.empty()) 
      return false;
  }

  if (size_t(position + nr_output_samples +1) >= input_buffer_->front().data().size()) {
    if (input_buffer_->size() < 2) {
      return false;
    }
  }

  return true;
}

template <class Type>
void
Integer_delay_correction_all_channels<Type>::
connect_to(input_buffer_ptr buffer) {
  input_buffer_ = buffer;
}

template <class Type>
typename Integer_delay_correction_all_channels<Type>::output_buffer_ptr
Integer_delay_correction_all_channels<Type>::
get_output_buffer() {
  return output_buffer_;
}

template <class Type>
typename Integer_delay_correction_all_channels<Type>::Delay_type
Integer_delay_correction_all_channels<Type>::get_delay(int64_t time) {
  assert(delay_table.initialised());
  assert(subsamples_per_sample > 0);
  assert(delta_time > 0);
  assert(delta_time%2 == 0);
  double delay = delay_table.delay(time+delta_time/2);
  int delay_in_subsamples = (int)std::floor(delay*sample_rate*subsamples_per_sample+.5);

  // All because modulo doesn't work for negative values
  int sample_delay = -((-delay_in_subsamples)/subsamples_per_sample)-1;
  int subsample_delay = delay_in_subsamples-sample_delay*subsamples_per_sample;

  if (subsample_delay == subsamples_per_sample) {
    sample_delay++;
    subsample_delay = 0;
  }

  assert((0 <= subsample_delay) && (subsample_delay < subsamples_per_sample));
  assert((sample_delay*subsamples_per_sample + subsample_delay) == delay_in_subsamples);

  return Delay_type(sample_delay, subsample_delay);
}

template <class Type>
void
Integer_delay_correction_all_channels<Type>::
set_delay_table(Delay_table_akima &table) {
  delay_table = table;
}

template <class Type>
void
Integer_delay_correction_all_channels<Type>::
set_parameters(const Input_node_parameters &parameters,
               int node_nr) {
  node_nr_ = node_nr;

  bits_per_subsample = parameters.bits_per_sample();
  subsamples_per_sample = parameters.subsamples_per_sample();
  assert(parameters.number_channels%subsamples_per_sample == 0);

  nr_output_samples = parameters.number_channels/subsamples_per_sample;
  sample_rate = parameters.track_bit_rate;

  assert(nr_output_samples*1000000%sample_rate == 0);
  delta_time = nr_output_samples*1000000/sample_rate;
  integration_time = parameters.integr_time*1000;

  nr_bytes_per_integration_slice =
    Control_parameters::nr_bytes_per_integration_slice_input_node_to_correlator_node
    (parameters.integr_time,
     sample_rate*subsamples_per_sample,
     bits_per_subsample,
     parameters.number_channels);


}

template <class Type>
void
Integer_delay_correction_all_channels<Type>::
set_time(int64_t time) {
  assert(delay_table.initialised());
  current_time_ = time;
  current_delay = get_delay(current_time_);
  position = current_delay.first;
}

template <class Type>
void
Integer_delay_correction_all_channels<Type>::
set_stop_time(int64_t time) {
  stop_time_ = time;
}

template <class Type>
int
Integer_delay_correction_all_channels<Type>::
bytes_of_output(int nr_seconds) {
  if (nr_seconds < 0)
    return nr_seconds;

  int nr_time_slices = int64_t(1000)*nr_seconds/integration_time;

  return nr_bytes_per_integration_slice * nr_time_slices;
}

template <class Type>
void
Integer_delay_correction_all_channels<Type>::
pop_from_input_buffer() {
  input_buffer_->pop();
#ifdef SFXC_DETERMINISTIC
  { // Randomize header
    Input_memory_pool_element front = input_buffer_->front();
    for (int i=0; i<SIZE_MK4_HEADER; i++) {
      front.data()[i] = Type(0);
    }
  }
#else
  { // Randomize header
    Input_memory_pool_element front = input_buffer_->front();
    for (int i=0; i<SIZE_MK4_HEADER; i++) {
      // park_miller_random generates 31 random bits
      if (sizeof(Type) < 4) {
        front.data()[i] = (Type)park_miller_random();
      } else if (sizeof(Type) == 4) {
        front.data()[i] =
          (Type(park_miller_random())<<16) + park_miller_random();
      } else {
        assert(sizeof(Type) == 8);
        int64_t rnd = park_miller_random();
        rnd = (rnd << 16) + park_miller_random();
        rnd = (rnd << 16) + park_miller_random();
        rnd = (rnd << 16) + park_miller_random();
        front.data()[i] = rnd;
      }
    }
  }
#endif
}

#endif // INTEGER_DELAY_CORRECTION_ALL_CHANNELS_H
