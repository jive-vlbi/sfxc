#ifndef CORRELATION_CORE_H_
#define CORRELATION_CORE_H_

#include "tasklet/tasklet.h"
#include "delay_correction.h"
#include "Control_parameters.h"
#include "Data_writer.h"
#include "Timer.h"

class Correlation_core : public Tasklet
{
public:
  typedef Delay_correction::Output_buffer_element        Input_buffer_element;
  typedef Delay_correction::Output_buffer                Input_buffer;
  typedef Delay_correction::Output_buffer_ptr            Input_buffer_ptr;

  typedef Buffer_element<char,131072>                    Output_buffer_element;
  //typedef Buffer_element_large<DOUBLE,131072>          Output_buffer_element;
  typedef Semaphore_buffer<Output_buffer_element>        Output_buffer;
  typedef boost::shared_ptr<Output_buffer>               Output_buffer_ptr;
  
  Correlation_core();
  virtual ~Correlation_core();
  
  /// For Tasklet
  void do_task();
  
  bool finished();
  
  void connect_to(size_t stream, Input_buffer_ptr buffer);
  
  void set_parameters(const Correlation_parameters &parameters);
  void set_data_writer(boost::shared_ptr<Data_writer> writer);
  
  Output_buffer_ptr get_output_buffer();
  
private:
  bool is_ready_for_do_task();
  
  void integration_initialise();
  void integration_step();
  void integration_average();
  void integration_write();

  void auto_correlate_baseline(std::complex<DOUBLE> in[],
                               std::complex<DOUBLE> out[]);

  void correlate_baseline(std::complex<DOUBLE> in1[],
                          std::complex<DOUBLE> in2[],
                          std::complex<DOUBLE> out[]);

  
  size_t size_of_fft();
  size_t n_channels();
  size_t n_stations();

private:
  std::vector<Input_buffer_ptr>  input_buffers;
  Output_buffer_ptr              output_buffer;
  Output_buffer_element          *output_element;

  int number_ffts_in_integration, current_fft;
  
  Correlation_parameters             correlation_parameters;
  
  std::vector< std::vector< std::complex<DOUBLE> > > frequency_buffer;
  std::vector< std::complex<DOUBLE> > accumulation_buffers;
  std::vector< std::pair<int, int> >  baselines;
  
  boost::shared_ptr<Data_writer>      writer;

  Timer timer;
};

#endif /*CORRELATION_CORE_H_*/
