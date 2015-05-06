#ifndef DEDISPERSION_TASKLET_H
#define DEDISPERSION_TASKLET_H
#include "utils.h"
#include "control_parameters.h"
#include "correlator_node_types.h"
#include "coherent_dedispersion.h"
#ifdef USE_DOUBLE
#include "sfxc_fft.h"
#else
#include "sfxc_fft_float.h"
#endif

class Dedispersion_tasklet{
public:
  typedef Correlator_node_types::Delay_queue_ptr     Delay_queue_ptr;
  typedef boost::shared_ptr<Coherent_dedispersion>   Coherent_dedispersion_ptr;
  typedef Pulsar_parameters::Pulsar                  Pulsar;

  Dedispersion_tasklet();
  ~Dedispersion_tasklet();
  bool do_task();
  void set_parameters(const Correlation_parameters &parameters, Pulsar &pulsar);
  void empty_output_queue();
  void connect_to(Delay_queue_ptr buffer, int stream_nr);
  /// Get the output
  Delay_queue_ptr get_output_buffer(int stream_nr);
private:
  void create_dedispersion_filter();
private:
  double channel_freq, channel_bw; // In MHz
  double sample_rate;
  Time integer_channel_offset;
  double channel_offset, DM;
  int sideband;
  int out_pos;
  int current_fft, current_buffer;
  int fft_size_dedispersion, fft_size_correlation;
  int total_input_fft; // FIXME debug info
  Time current_time, start_time, stop_time;
  Memory_pool_vector_element<std::complex<FLOAT> > filter, dedispersion_buffer;
  Memory_pool_vector_element<FLOAT> zeropad_buffer;
  std::vector<Coherent_dedispersion_ptr>  dedispersion_modules;
  SFXC_FFT  fft;
};
#endif
