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
  typedef Correlator_node_types::Bit_sample_queue     Input_buffer;
  typedef Correlator_node_types::Bit_sample_queue_ptr Input_buffer_ptr;
  typedef Input_buffer::value_type                    Input_buffer_element;

  Delay_correction_base();
  virtual ~Delay_correction_base();

  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);

  /// Set the delay table
  void set_delay_table(const Delay_table_akima &delay_table);

  virtual void set_parameters(const Correlation_parameters &parameters)=0;

  /// Do one delay step
  virtual void do_task()=0;

  virtual bool has_work()=0;
  const char *name() {
    return __PRETTY_FUNCTION__;
  }
private:
  ///
  void bit2float(const Input_buffer_element &input, FLOAT *output_buffer);

private:
  // access functions to the correlation parameters
  size_t number_channels();
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

  int n_ffts_per_integration, current_fft, total_ffts;

  FFTW_PLAN          plan_t2f, plan_f2t;

  bool delay_table_set;
  Delay_table_akima   delay_table;

  Memory_pool_vector_element< std::complex<FLOAT> > frequency_buffer;
  std::vector< FLOAT > time_buffer;

  Timer delay_timer;

  FLOAT lookup_table[256][4];
};

#endif /*DELAY_CORRECTION_BASE_H*/
