#ifndef DELAY_CORRECTION_H
#define DELAY_CORRECTION_H

#include <complex>
#include <fftw3.h>

#include "tasklet/tasklet.h"
#include "delay_table_akima.h"
#include "bits_to_float_converter.h"
#include "control_parameters.h"

#include "timer.h"

class Delay_correction : public Tasklet
{
public:
  typedef Bits_to_float_converter::Output_buffer_element Input_buffer_element;
  typedef Bits_to_float_converter::Output_buffer         Input_buffer;
  typedef Bits_to_float_converter::Output_buffer_ptr     Input_buffer_ptr;

  typedef Buffer_element_vector<FLOAT>                  Output_buffer_element;
  typedef Semaphore_buffer<Output_buffer_element>        Output_buffer;
  typedef boost::shared_ptr<Output_buffer>               Output_buffer_ptr;

  
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
  
  bool is_ready_for_do_task();
  
private:
  void fractional_bit_shift(std::complex<FLOAT> output[],
                            int integer_shift,
                            FLOAT fractional_delay);
  void fringe_stopping(std::complex<FLOAT> intput[],
                       FLOAT output[]);

private:
  // access functions to the correlation parameters
  int number_channels();
  int sample_rate();
  int bandwidth();
  int length_of_one_fft(); // Length of one fft in microseconds 
  int sideband();
  int64_t channel_freq();
  FLOAT get_delay(int64_t time);

private:
  Input_buffer_ptr    input_buffer;
  Output_buffer_ptr   output_buffer;
  
  int64_t             current_time; // In microseconds
  Correlation_parameters correlation_parameters;
  

  FFTW_PLAN           plan_t2f_orig, plan_f2t_orig, plan_t2f, plan_f2t;
  std::vector<FLOAT> freq_scale; // frequency scale for the fractional bit shift
  // For fringe stopping we do a linear approximation
  // maximal_phase_change is the maximal angle between two
  // sample points
  static const FLOAT maximal_phase_change; // 5.7 degrees
  int n_recompute_delay;
     
  bool delay_table_set;
  Delay_table_akima   delay_table;
  
  // You need this one because the input and output are FLOATs (not complex)
  std::vector<std::complex<FLOAT> > frequency_buffer;
  
  // Buffer for the integer bit shift
  std::vector<FLOAT> intermediate_buffer;
  
  Timer delay_timer;
public:
  bool verbose;
};

#endif /*DELAY_CORRECTION_H*/
