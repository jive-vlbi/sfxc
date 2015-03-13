/* Copyright (c) 2009 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Aard Keimpema, <keimpema@jive.nl>
 *
 */

#ifndef BIT2FLOAT_WORKER
#define BIT2FLOAT_WORKER

#include <limits.h>
#include "correlator_node_types.h"
#include "utils.h"
#include "delay_table_akima.h"
#include "control_parameters.h"
#include "bit_statistics.h"

class Bit2float_worker;
typedef boost::shared_ptr<Bit2float_worker> Bit2float_worker_sptr;

class Bit2float_worker  {
public:
  typedef Correlator_node_types Types;

  // Input data from correlator_node_data_reader_tasklet
  typedef Types::Channel_circular_input_buffer      Input_buffer;
  typedef Types::Channel_circular_input_buffer_ptr  Input_buffer_ptr;

  // Output data to the delay modules
  typedef Types::Channel_memory_pool                Output_memory_pool;
  typedef Types::Channel_memory_pool_element        Output_pool_element;
  typedef Types::Channel_memory_pool_data           Output_pool_data;
  typedef Types::Channel_queue                      Output_queue;
  typedef Types::Channel_queue_ptr                  Output_queue_ptr;

  // Invalid data
  typedef Correlator_node_types::Invalid            Invalid;

  Bit2float_worker(int stream_nr, bit_statistics_ptr statistics_);
  ~Bit2float_worker() {
    // We need to make sure that output_element is released
    Output_pool_element dummy;
    out_element=dummy;
  }

  /// For tasklet

  /// Process one piece of data, return the amount of data processed
  int64_t do_task();

  /// Check if we can process data
  bool has_work();
  /// Set the input
  void connect_to(Input_buffer_ptr new_Input_buffer);
  /// Get the output
  const char *name() {
    return "Bit2float_worker";
  }

  Output_queue_ptr get_output_buffer();

  void set_new_parameters(const Correlation_parameters &parameters, Delay_table_akima &delays);

  // Convert input bitstream to floating point
  int bit2float(FLOAT *output, int start, int nbits, uint64_t *read);

  // Empty the input queue, called from the destructor of Input_node
  // This is necessary because of pre-extracting data
  void empty_input_queue();
  
  // Obtain the list of invalid samples
  std::vector<Invalid> *get_invalid();

  static Bit2float_worker_sptr new_sptr(int stream_nr_, bit_statistics_ptr statistics_);
private:
  // get the position index of the next delay change / invalid block
  int get_next_delay();
  int get_next_invalid_block();
  void set_parameters();
  void allocate_element(); // Allocate a new output packet
  Output_pool_element out_element; // The current output packet
  int out_index; // Current index in the data buffer of out_element

  Input_buffer_ptr    input_buffer_;
  Output_queue_ptr    output_buffer_;
  Output_memory_pool   memory_pool_;

  /// The lookup tables for the bit2float conversion
  FLOAT lookup_table[256][4];
  FLOAT lookup_table_1bit[256][8];
  bit_statistics_ptr statistics;
  int tsys_count;
  bool tsys_on;

  int state;
  enum {IDLE, SEND_INVALID, SEND_DATA, PURGE_STREAM};
  size_t bytes_left;
  size_t invalid_left;
  /// indicates starting which sample in the current input byte we should start writing
  size_t sample_in_byte;
  /// The total number of FFT's in current time slice
  int n_ffts_per_integration;
  /// The current fft being processed
  int current_fft;
  /// The number of ffts per buffer
  int nfft_max;
  /// The number of samples per FFT
  int fft_size;
  /// Number of bits per sample
  int bits_per_sample;
  /// Data rate [samples/sec]
  int sample_rate;
  int base_sample_rate;
  /// index of the current stream in the station list
  int stream_nr;
  int16_t invalid_samples;
  int8_t cur_delay;
  /// Indicates if we have received a new set of parameters
  bool have_new_parameters;
  struct New_parameters{
    int n_ffts_per_integration;
    int n_ffts_per_buffer;
    int bits_per_sample;
    int sample_rate;
    int base_sample_rate;
    int fft_size_delaycor;
    int fft_size_correlation;
    int delay_in_samples;
    Time start_time;
  } new_parameters;
  /// List of all invalid samples in the time slice
  std::vector<Invalid> invalid;
};
#endif // INTEGER_DELAY_CORRECTION_PER_CHANNEL_H
