/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Aard Keimpema <keimpema@jive.nl>, 2008
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef DELAY_CORRECTION_BASE_H
#define DELAY_CORRECTION_BASE_H
#include <boost/shared_ptr.hpp>
#include <complex>
#include <fftw3.h>

#include "tasklet/tasklet.h"
#include "delay_table_akima.h"
#include "correlator_node_types.h"
#include "control_parameters.h"

#include "timer.h"

class Delay_correction_default;
class Delay_correction_swapped;

class Delay_correction_base : public Tasklet {
friend class Delay_correction_default;
friend class Delay_correction_swapped;
public:

  typedef Correlator_node_types::Channel_queue      Input_buffer;
  typedef Correlator_node_types::Channel_queue_ptr  Input_buffer_ptr;
  typedef Input_buffer::value_type                  Input_buffer_element;

  typedef Correlator_node_types::ComplexFloat_memory_pool  Output_memory_pool;
  typedef Correlator_node_types::ComplexFloat_element      Output_data;
  typedef Correlator_node_types::ComplexFloat_queue        Output_buffer;
  typedef Correlator_node_types::ComplexFloat_queue_ptr    Output_buffer_ptr;
  typedef Output_buffer::value_type                        Output_buffer_element;

  Delay_correction_base(int stream_nr);
  virtual ~Delay_correction_base();

  /// Get the output
  Output_buffer_ptr get_output_buffer();

  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);

  /// Set the delay table
  void set_delay_table(const Delay_table_akima &delay_table);

  virtual void set_parameters(const Correlation_parameters &parameters)=0;

  /// Do one delay step
  virtual void do_task()=0;

  bool has_work();
  const char *name() {
    return __PRETTY_FUNCTION__;
  }

private:
  // access functions to the correlation parameters
  size_t number_channels();
  size_t size_of_fft();
  int buffer_size;
  int sample_rate();
  int bandwidth();
  int length_of_one_fft(); // Length of one fft in microseconds
  int sideband();
  int64_t channel_freq();
  double get_delay(int64_t time);
private:
  Input_buffer_ptr    input_buffer;

  int64_t             current_time; // In microseconds
  Correlation_parameters correlation_parameters;
  int	stream_nr;
  int	bits_per_sample;
  int oversamp; // The amount of oversampling

  int n_ffts_per_integration, current_fft, total_ffts;
  int nfft_max; // The maximum number of fft's in an input frame
  bool delay_table_set;
  Delay_table_akima   delay_table;

  Memory_pool_vector_element< std::complex<FLOAT> > frequency_buffer;
  Memory_pool_vector_element<FLOAT> time_buffer;

  Timer delay_timer;

private:
  Output_buffer_ptr   output_buffer;
  Output_memory_pool  output_memory_pool;
  Output_buffer_element cur_output;

};

inline size_t Delay_correction_base::number_channels() {
  return correlation_parameters.number_channels;
}

inline size_t Delay_correction_base::size_of_fft() {
  return 2*correlation_parameters.number_channels;
}

inline int Delay_correction_base::bandwidth() {
  return correlation_parameters.bandwidth;
}
inline int Delay_correction_base::sample_rate() {
  return correlation_parameters.sample_rate;
}

inline int64_t Delay_correction_base::channel_freq() {
  return correlation_parameters.channel_freq;
}


#endif /*DELAY_CORRECTION_BASE_H*/
