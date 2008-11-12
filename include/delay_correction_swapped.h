/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Aard Keimpema <keimpema@jive.nl> 2008
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef DELAY_CORRECTION_SWAPPED_H
#define DELAY_CORRECTION_SWAPPED_H
#include <boost/shared_ptr.hpp>
#include <complex>
#include <fftw3.h>

#include "delay_correction_base.h"
#include "tasklet/tasklet.h"
#include "delay_table_akima.h"
#include "correlator_node_types.h"
#include "control_parameters.h"

#include "timer.h"

class Delay_correction_swapped : public Delay_correction_base {
public:
  // Types used when order frac. bitshift and fringe rot. is reversed
  typedef Correlator_node_types::ComplexFloat_memory_pool  Output_memory_pool_cmplx;
  typedef Correlator_node_types::ComplexFloat_queue        Output_buffer_cmplx;
  typedef Correlator_node_types::ComplexFloat_queue_ptr    Output_buffer_cmplx_ptr;
  typedef Output_buffer_cmplx::value_type                  Output_buffer_cmplx_element;

  Delay_correction_swapped();
  ~Delay_correction_swapped(){};

  /// Get the output
  Output_buffer_cmplx_ptr get_output_buffer();

  bool has_work();
  void set_parameters(const Correlation_parameters &parameters);
  /// Do one delay step
  void do_task();

private:
  ///
  void fractional_bit_shift(std::complex<FLOAT> input[],
                            int integer_shift,
                            FLOAT fractional_delay);
  void fringe_stopping(FLOAT output[]);

private:
  Output_buffer_cmplx_ptr   output_buffer;
  Output_memory_pool_cmplx  output_memory_pool;

};

#endif /*DELAY_CORRECTION_SWAPPED_H*/
