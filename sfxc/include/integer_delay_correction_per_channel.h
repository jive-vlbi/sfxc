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

#include "input_node_types.h"
#include "utils.h"
#include "delay_table_akima.h"
#include "control_parameters.h"

class Integer_delay_correction_per_channel {
public:
  // Input data, this comes from the channel extractor
  typedef Input_node_types::Channel_buffer              Input_buffer;
  typedef Input_node_types::Channel_buffer_element      Input_buffer_element;
  typedef Input_node_types::Channel_buffer_ptr          Input_buffer_ptr;
  typedef Input_node_types::Channel_memory_pool         Input_memory_pool;
  typedef Input_node_types::Channel_memory_pool_element Input_memory_pool_element;

  // Output data goes to the Input_node_data_writer
  typedef Input_node_types::Channel_memory_pool         Output_memory_pool;
  typedef Input_node_types::Fft_buffer_element          Output_buffer_element;
  typedef Input_node_types::Fft_buffer                  Output_buffer;
  typedef Input_node_types::Fft_buffer_ptr              Output_buffer_ptr;
  typedef Input_node_types::Channel_memory_pool_element Input_data_block;

  // Pair of the delay in samples (of type Type) and subsamples
  // - first is the delay in bytes
  // - second is the number of samples in the byte that should be shifted
  // Hence, integer bit shift = first*(8/bits_per_sample) + second
  typedef std::pair<int,int>                            Delay_type;

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

  // Set the start time in microseconds
  // This might be different from the time from which data arrives.
  // If no data is available, invalid/random data will be sent
  void set_time(int64_t time);
  // Setting of the stop time in microseconds
  void set_stop_time(int64_t time);
  // Initialisation of the delay table
  void set_delay_table(Delay_table_akima &table);
  // Set the information for one integration slice, some information is
  // redundant (e.g. the bits/sample will not change during an experiment)
  void set_parameters(const Input_node_parameters &parameters, int node_nr);

  // Empty the input queue, called from the destructor of Input_node
  // This is necessary because of pre-extracting data
  void empty_input_queue();
  
  // Number of bytes per integration slice
  int bytes_of_output();
private:

  // Checks whether the start time has been set
  bool time_set() const {
    return _current_time > 0;
  }
  // Returns the time we are currently processing
  int64_t current_time() const {
    return _current_time;
  }
  // Returns the delay in seconds for the time in microseconds 
  double delay(int64_t time);
  
  // Returns the delay in terms of samples, time is in microseconds
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

#endif // INTEGER_DELAY_CORRECTION_PER_CHANNEL_H
