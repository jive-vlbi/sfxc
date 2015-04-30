#ifndef COHERENT_DEDISPERSION_H
#define COHERENT_DEDISPERSION_H
#include "utils.h"
#include "control_parameters.h"
#include "correlator_node_types.h"
#ifdef USE_DOUBLE
#include "sfxc_fft.h"
#else
#include "sfxc_fft_float.h"
#endif

class Coherent_dedispersion{
public:
  typedef Correlator_node_types::Delay_memory_pool   Delay_memory_pool;
  typedef Correlator_node_types::Delay_queue         Delay_queue;
  typedef Correlator_node_types::Delay_queue_ptr     Delay_queue_ptr;
  typedef Delay_queue::value_type                    Delay_queue_element;
  typedef Pulsar_parameters::Pulsar                  Pulsar;

  Coherent_dedispersion(int stream_nr_, SFXC_FFT &fft_, 
                      Memory_pool_vector_element<std::complex<FLOAT> > &filter_,
                      Memory_pool_vector_element<std::complex<FLOAT> > &dispersion_buffer_,
                      Memory_pool_vector_element<FLOAT> &zeropad_buffer_ );
  ~Coherent_dedispersion();
  void do_task();
  bool has_work();
  void empty_output_queue();
  void set_parameters(const Correlation_parameters &parameters);
  void connect_to(Delay_queue_ptr buffer);
  /// Get the output
  Delay_queue_ptr get_output_buffer();
private:
  void allocate_element(int nfft);
  void overlap_add();
private:
  int stream_nr;
  int out_pos;
  int current_fft, current_buffer;
  int fft_size_dedispersion, fft_size_correlation;
  int total_input_fft; // FIXME debug info
  double sample_rate;
  Time current_time, start_time, stop_time;
  Memory_pool_vector_element<std::complex<FLOAT> > &filter, &dedispersion_buffer;
  Memory_pool_vector_element<FLOAT> time_buffer[2], &zeropad_buffer;
  Delay_queue_element cur_output;

  Delay_memory_pool output_memory_pool;
  Delay_queue_ptr input_queue;
  Delay_queue_ptr output_queue;
  SFXC_FFT  &fft;
};
#endif
