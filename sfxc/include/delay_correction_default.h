/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Aard Keimpema <keimpema@jive.nl>, 2008
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef DELAY_CORRECTION_DEFAULT_H
#define DELAY_CORRECTION_DEFAULT_H
#include <boost/shared_ptr.hpp>
#include <complex>
#include <fftw3.h>

#include "delay_correction_base.h"
#include "tasklet/tasklet.h"
#include "delay_table_akima.h"
#include "correlator_node_types.h"
#include "control_parameters.h"

#include "timer.h"

class Delay_correction_default : public Delay_correction_base {
public:
  typedef Correlator_node_types::Float_memory_pool    Output_memory_pool;
  typedef Correlator_node_types::Float_queue          Output_buffer;
  typedef Correlator_node_types::Float_queue_ptr      Output_buffer_ptr;
  typedef Output_buffer::value_type                   Output_buffer_element;


  Delay_correction_default();
  ~Delay_correction_default(){};

  /// Get the output
  Output_buffer_ptr get_output_buffer();

  bool has_work();
  void set_parameters(const Correlation_parameters &parameters);
  /// Do one delay step
  void do_task();


private:
  ///
  void fractional_bit_shift(FLOAT input[],
                            int integer_shift,
                            FLOAT fractional_delay);
  void fringe_stopping(FLOAT output[]);

private:
  Output_buffer_ptr   output_buffer;
  Output_memory_pool  output_memory_pool;
};

#endif /*DELAY_CORRECTION_DEFAULT_H*/
