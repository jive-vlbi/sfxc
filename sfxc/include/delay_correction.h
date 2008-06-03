/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef DELAY_CORRECTION_H
#define DELAY_CORRECTION_H
#include <boost/shared_ptr.hpp>
#include <complex>
#include <fftw3.h>

#include "tasklet/tasklet.h"
#include "semaphore_buffer.h"
#include "delay_table_akima.h"
#include "correlator_node_types.h"
#include "control_parameters.h"

#include "timer.h"

class Delay_correction : public Tasklet {
public:
  typedef Correlator_node_types::Bit_sample_queue     Input_buffer;
  typedef Correlator_node_types::Bit_sample_queue_ptr Input_buffer_ptr;
  typedef Input_buffer::value_type                    Input_buffer_element;

  typedef Correlator_node_types::Float_memory_pool    Output_memory_pool;
  typedef Correlator_node_types::Float_queue          Output_buffer;
  typedef Correlator_node_types::Float_queue_ptr      Output_buffer_ptr;
  typedef Output_buffer::value_type                   Output_buffer_element;


  Delay_correction();
  virtual ~Delay_correction();

  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);

  /// Get the output
  Output_buffer_ptr get_output_buffer();

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
  ///
  void bit2float(const Input_buffer_element &input, FLOAT* output_buffer);
  void fractional_bit_shift(FLOAT input[],
                            int integer_shift,
                            FLOAT fractional_delay);
  void fringe_stopping(FLOAT output[]);

private:
  // access functions to the correlation parameters
  int number_channels();
  int sample_rate();
  int bandwidth();
  int length_of_one_fft(); // Length of one fft in microseconds
  int sideband();
  int64_t channel_freq();
  double get_delay(int64_t time);

private:
  Input_buffer_ptr    input_buffer;
  Output_buffer_ptr   output_buffer;
  Output_memory_pool  output_memory_pool;

  int64_t             current_time; // In microseconds
  Correlation_parameters correlation_parameters;

  int n_ffts_per_integration, current_fft, total_ffts;

  FFTW_PLAN          plan_t2f, plan_f2t;
  // buffer used for the plan
  std::vector<std::complex<FLOAT> > buffer;

  bool delay_table_set;
  Delay_table_akima   delay_table;

  // You need this one because the input and output are FLOATs (not complex)
  Aligned_vector< std::complex<FLOAT> > frequency_buffer;

  Timer delay_timer;

  FLOAT lookup_table[256][4];
};

#endif /*DELAY_CORRECTION_H*/
