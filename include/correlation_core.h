#ifndef CORRELATION_CORE_H_
#define CORRELATION_CORE_H_

#include "tasklet/tasklet.h"
#include "delay_correction.h"
#include "control_parameters.h"
#include "data_writer.h"

#include "timer.h"

class Correlation_core : public Tasklet
{
public:
  typedef Delay_correction::Output_buffer_element        Input_buffer_element;
  typedef Delay_correction::Output_buffer                Input_buffer;
  typedef Delay_correction::Output_buffer_ptr            Input_buffer_ptr;

  typedef Buffer_element<char,131072>                    Output_buffer_element;
  //typedef Buffer_element_large<FLOAT,131072>          Output_buffer_element;
  typedef Semaphore_buffer<Output_buffer_element>        Output_buffer;
  typedef boost::shared_ptr<Output_buffer>               Output_buffer_ptr;
  
  Correlation_core();
  virtual ~Correlation_core();
  
  /// For Tasklet
  void do_task();
  bool has_work();
  const char *name() { return __PRETTY_FUNCTION__; }
  
  bool finished();
  
  void connect_to(size_t stream, Input_buffer_ptr buffer);
  
  void set_parameters(const Correlation_parameters &parameters,
                      int node_nr);
  void set_data_writer(boost::shared_ptr<Data_writer> writer);
  
  Output_buffer_ptr get_output_buffer();

  int number_of_baselines() {
    return baselines.size();
  }
  boost::shared_ptr<Data_writer> data_writer() {
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

private:
  std::vector<Input_buffer_ptr>  input_buffers;
  Output_buffer_ptr              output_buffer;
  Output_buffer_element          *output_element;

  // Used in integration_step(), avoids contruction and destroying the vectors
  std::vector<Input_buffer_element *>               input_elements;


  int number_ffts_in_integration, current_fft, total_ffts;

  FFTW_PLAN                          plan;
  std::vector< FLOAT >               plan_input_buffer;
  std::vector< std::complex<FLOAT> > plan_output_buffer;
  
  Correlation_parameters                            correlation_parameters;
  
  std::vector< std::vector< std::complex<FLOAT> > > frequency_buffer;
  std::vector< std::complex<FLOAT> >                accumulation_buffers;
  std::vector< std::complex<float> >        accumulation_buffers_float;  
  std::vector< std::pair<int, int> >                baselines;
  
  boost::shared_ptr<Data_writer>                    writer;

  Timer fft_timer;

  // Needed for writing the progress messages
  int node_nr_;
};

#endif /*CORRELATION_CORE_H_*/
