#ifndef WINDOWING_H
#define WINDOWING_H
#include "utils.h"
#include "control_parameters.h"
#include "correlator_node_types.h"
#ifdef USE_DOUBLE
#include "sfxc_fft.h"
#else
#include "sfxc_fft_float.h"
#endif

class Windowing{
public:
  typedef Correlator_node_types::Correlation_memory_pool   Correlation_memory_pool;
  typedef Correlator_node_types::Correlation_queue         Correlation_queue;
  typedef Correlator_node_types::Correlation_queue_ptr     Correlation_queue_ptr;
  typedef Correlation_queue::value_type                    Correlation_queue_element;
  typedef Correlator_node_types::Delay_memory_pool   Delay_memory_pool;
  typedef Correlator_node_types::Delay_queue         Delay_queue;
  typedef Correlator_node_types::Delay_queue_ptr     Delay_queue_ptr;
  typedef Delay_queue::value_type                    Delay_queue_element;

  Windowing(int stream_nr_);
  ~Windowing();
  void do_task();
  bool has_work();
  void empty_output_queue();
  void set_parameters(const Correlation_parameters &parameters);
  void connect_to(Delay_queue_ptr buffer);
  /// Get the output
  Correlation_queue_ptr get_output_buffer();
private:
  Correlation_queue_element allocate_element(int nfft);
  void create_window();
private:
  int index;
  int samples_to_skip;
  int stream_nr;
  int window_func;
  int output_stride;
  int current_fft, current_buffer;
  int fft_rot_size, fft_size_correlation;
  int nffts_per_integration;
  Time current_time, integration_start;

  Memory_pool_vector_element<FLOAT> window;
  Memory_pool_vector_element<FLOAT> buffers[2];
  Memory_pool_vector_element< std::complex<FLOAT> > temp_fft_buffer;
  int buf, temp_fft_offset, output_offset;

  Correlation_memory_pool output_memory_pool;
  Delay_queue_ptr input_queue;
  Correlation_queue_ptr output_queue;
  SFXC_FFT fft_t2f;
};
#endif
