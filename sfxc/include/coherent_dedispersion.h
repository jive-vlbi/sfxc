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

  Coherent_dedispersion(int stream_nr_);
  ~Coherent_dedispersion();
  void do_task();
  bool has_work();
  void empty_output_queue();
  void set_parameters(const Correlation_parameters &parameters, Pulsar &pulsar);
  void connect_to(Delay_queue_ptr buffer);
  /// Get the output
  Delay_queue_ptr get_output_buffer();
private:
  void allocate_element(int nfft);
  void create_dedispersion_filter();
  void overlap_add();
private:
  bool problem; // FOR TESTING 
  double channel_freq, channel_bw; // In MHz
  double sample_rate;
  Time integer_channel_offset;
  double channel_offset, DM;
  int stream_nr;
  int sideband;
  int out_pos, output_stride;
  int current_fft, current_buffer;
  int fft_size_dedispersion, fft_size_correlation;
  int nffts_per_integration;
  int total_input_fft; // FIXME debug info
  Time current_time, start_time, stop_time;
  Memory_pool_vector_element<std::complex<FLOAT> > filter, dedispersion_buffer;
  Memory_pool_vector_element<FLOAT> time_buffer[2], output_buffer;
  Delay_queue_element cur_output;

  Delay_memory_pool output_memory_pool;
  Delay_queue_ptr input_queue;
  Delay_queue_ptr output_queue;
  SFXC_FFT  fft_f2t, fft_t2f;
};
#endif
