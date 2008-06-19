#ifndef CORRELATION_CORE_H_
#define CORRELATION_CORE_H_

#include "tasklet/tasklet.h"
#include "delay_correction.h"
#include "control_parameters.h"
#include "data_writer.h"

#include "timer.h"
#include <fstream>

class Correlation_core : public Tasklet {
public:
  typedef Delay_correction::Output_buffer_element          Input_buffer_element;
  typedef Delay_correction::Output_buffer                  Input_buffer;
  typedef Delay_correction::Output_buffer_ptr              Input_buffer_ptr;

  typedef Memory_pool_vector_element<FLOAT>                Float_buffer;
  typedef Memory_pool_vector_element<std::complex<FLOAT> > Complex_buffer;
  typedef Memory_pool_vector_element<std::complex<float> > Complex_buffer_float;
  Correlation_core();
  virtual ~Correlation_core();

  /// For Tasklet
  void do_task();
  bool has_work();
  const char *name() {
    return __PRETTY_FUNCTION__;
  }

  bool finished();
  bool almost_finished();

  void connect_to(size_t stream, Input_buffer_ptr buffer);

  void set_parameters(const Correlation_parameters &parameters,
                      int node_nr);
  void set_data_writer(std::tr1::shared_ptr<Data_writer> writer);

  int number_of_baselines() {
    return baselines.size();
  }
  std::tr1::shared_ptr<Data_writer> data_writer() {
    return writer;
  }

private:
  void integration_initialise();
  void integration_step();
  void integration_average();
  void integration_write();

  void auto_correlate_baseline(std::complex<FLOAT> in[],
                               std::complex<FLOAT> out[]);

  void correlate_baseline(std::complex<FLOAT> in1[],
                          std::complex<FLOAT> in2[],
                          std::complex<FLOAT> out[]);


  size_t size_of_fft();
  size_t n_channels();
  size_t n_stations();

  size_t number_input_streams_in_use();

private:
  std::vector<Input_buffer_ptr>  input_buffers;

  // Used in integration_step(), avoids contruction and destroying the vectors
  std::vector<Input_buffer_element>     input_elements;

  int number_ffts_in_integration, current_fft, total_ffts;

  FFTW_PLAN       plan;
  Float_buffer    plan_input_buffer;
  Complex_buffer  plan_output_buffer;

  //std::vector< FLOAT >               plan_input_buffer;
  //std::vector< std::complex<FLOAT> > plan_output_buffer;


  Correlation_parameters                               correlation_parameters;

  std::vector< Complex_buffer >                        frequency_buffer;
  std::vector< Complex_buffer >                        accumulation_buffers;
  Complex_buffer_float                                 accumulation_buffers_float;
  std::vector< std::pair<size_t, size_t> >             baselines;

  std::tr1::shared_ptr<Data_writer>                       writer;

  Timer fft_timer;

  // Needed for writing the progress messages
  int node_nr_;
  int current_integration;

#ifdef SFXC_WRITE_STATS
  // For plotting statistics on the height of the fringe and the phase
  std::ofstream                                        stats_out;
  Complex_buffer                                       backward_buffer;
  FFTW_PLAN                                            backward_plan_;
#endif // SFXC_WRITE_STATS

};

#endif /*CORRELATION_CORE_H_*/
