/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Aard Keimpema <keimpema@jive.nl>, 2008
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef DELAY_CORRECTION_H
#define DELAY_CORRECTION_H
#include <boost/shared_ptr.hpp>
#include <complex>

#include "delay_correction.h"
#include "tasklet/tasklet.h"
#include "delay_table_akima.h"
#include "correlator_node_types.h"
#include "control_parameters.h"
#include "timer.h"
#include "utils.h"
#ifdef USE_DOUBLE
#include "sfxc_fft.h"
#else
#include "sfxc_fft_float.h"
#endif
class Delay_correction {
public:
  typedef Correlator_node_types::Channel_queue       Input_buffer;
  typedef Correlator_node_types::Channel_queue_ptr   Input_buffer_ptr;
  typedef Input_buffer::value_type                   Input_buffer_element;

  typedef Correlator_node_types::Delay_memory_pool   Output_memory_pool;
  typedef Correlator_node_types::Delay_queue         Output_buffer;
  typedef Correlator_node_types::Delay_queue_ptr     Output_buffer_ptr;
  typedef Output_buffer::value_type                  Output_buffer_element;

  Delay_correction(int stream_nr);
  ~Delay_correction();

  /// Get the output
  Output_buffer_ptr get_output_buffer();

  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);

  /// Set the delay table
  void set_delay_table(const Delay_table_akima &delay_table);

  void set_parameters(const Correlation_parameters &parameters);
  /// Do one delay step
  void do_task();
  bool has_work();
  const char *name() {
    return __PRETTY_FUNCTION__;
  }
private:
  void fractional_bit_shift(FLOAT *input,
                            int integer_shift,
                            double fractional_delay);
  void fringe_stopping(FLOAT output[]);
  // access functions to the correlation parameters
  size_t fft_size();
  size_t fft_cor_size();
  int buffer_size;
  int sample_rate();
  int bandwidth();
  int sideband();
  int64_t channel_freq();
  double get_delay(Time time);
  double get_phase(Time time);
  double get_amplitude(Time time);
  void create_window();
  void create_flip();

private:
  Input_buffer_ptr    input_buffer;

  Time             current_time;
  Correlation_parameters correlation_parameters;
  int   stream_nr;
  int   stream_idx;
  int   bits_per_sample;
  int oversamp; // The amount of oversampling

  int n_ffts_per_integration, current_fft, total_ffts;
  size_t tbuf_start, tbuf_end;
  bool delay_table_set;
  Delay_table_akima   delay_table;

  Memory_pool_vector_element< std::complex<FLOAT> > frequency_buffer;
  Memory_pool_vector_element<FLOAT> time_buffer;
  Memory_pool_vector_element<FLOAT> temp_buffer;
  Memory_pool_vector_element<FLOAT> window;
  Memory_pool_vector_element<FLOAT> flip;
  
  Timer delay_timer;

  Output_buffer_ptr   output_buffer;
  Output_memory_pool  output_memory_pool;
  Output_buffer_element cur_output;

  Time fft_length;
  SFXC_FFT        fft_t2f, fft_f2t, fft_t2f_cor;
  Memory_pool_vector_element< std::complex<FLOAT> >  exp_array;
};

inline size_t Delay_correction::fft_size() {
  return correlation_parameters.fft_size_delaycor;
}

inline size_t Delay_correction::fft_cor_size() {
  return 2*correlation_parameters.fft_size_correlation;
}

inline int Delay_correction::bandwidth() {
  return correlation_parameters.bandwidth;
}
inline int Delay_correction::sample_rate() {
  return correlation_parameters.sample_rate;
}

inline int64_t Delay_correction::channel_freq() {
  return correlation_parameters.station_streams[stream_idx].channel_freq;
}


#endif /*DELAY_CORRECTION_H*/
