#ifndef CORRELATION_CORE_H_
#define CORRELATION_CORE_H_

#include "sfxc_math.h"
#include "sfxc_mpi.h"
#include "tasklet/tasklet.h"
#include "delay_correction.h"
#include "control_parameters.h"
#include "data_writer.h"
#include "uvw_model.h"
#include "bit_statistics.h"
#include "timer.h"
#include <fstream>

class Correlation_core : public Tasklet {
//friend class Correlation_core_pulsar;
public:
  typedef Delay_correction::Output_buffer_element       Input_buffer_element;
  typedef Delay_correction::Output_buffer               Input_buffer;
  typedef Delay_correction::Output_buffer_ptr           Input_buffer_ptr;
  typedef Correlator_node_types::Invalid                Invalid;

  typedef Memory_pool_vector_element<std::complex<FLOAT> > Complex_buffer;
  typedef Memory_pool_vector_element<std::complex<float> > Complex_buffer_float;
  typedef Memory_pool_vector_element<FLOAT> Real_buffer;

  Correlation_core();
  virtual ~Correlation_core();

  /// For Tasklet
  virtual void do_task();
  bool has_work();
  const char *name() {
    return __PRETTY_FUNCTION__;
  }

  bool finished();
  bool almost_finished();

  void connect_to(size_t stream, bit_statistics_ptr statistics_, Input_buffer_ptr buffer);
  void connect_to(size_t stream, std::vector<Invalid> *invalid_);

  virtual void set_parameters(const Correlation_parameters &parameters,
                              int node_nr);
  void create_baselines(const Correlation_parameters &parameters);
  void set_data_writer(boost::shared_ptr<Data_writer> writer);

  int number_of_baselines() {
    return baselines.size();
  }
  boost::shared_ptr<Data_writer> data_writer() {
    return writer;
  }

  void add_uvw_table(int sn, Uvw_model &table);
  std::vector< Uvw_model>  uvw_tables; // Should be private
  void add_delay_table(int sn, Delay_table_akima &table);
  std::vector<Delay_table_akima> delay_tables;
  void add_source_list(const std::map<std::string, int> &sources_);

protected:
  virtual void integration_initialise();
  void integration_step(std::vector<Complex_buffer> &integration_buffer, int nbuffer, int stride);
  void integration_normalize(std::vector<Complex_buffer> &integration_buffer);
  void integration_write(std::vector<Complex_buffer> &integration_buffer, int phase_center, int bin);
  void tsys_write();
  void sub_integration();
  void find_invalid();
  void get_input_streams();

  void uvshift(const Complex_buffer &input_buffer, Complex_buffer &output_buffer, double ddelay1,
               double ddelay2, double rate1, double rate2);

  size_t number_channels();
  size_t fft_size();
  size_t number_input_streams();

  size_t number_input_streams_in_use();
  void create_window();

protected:
  int previous_fft;
  std::vector<Input_buffer_ptr>           input_buffers;
  std::vector< std::complex<FLOAT> * >    input_elements;
  std::vector< std::vector<Invalid> * >   invalid_elements;
  // the complex conjugate of input_elements
  std::vector< Complex_buffer >           input_conj_buffers; 
  std::vector<bit_statistics_ptr>         statistics;
  std::vector<int>                        streams_in_scan;
  // Tracks the number of correlator points where one (but not both) stations on a baseline had invalid data
  std::vector< std::pair<int64_t,int64_t> > n_flagged;

  Correlation_parameters                               correlation_parameters;
  int                                                  oversamp; // Oversample factor
  // A list of all sources in the current job
  std::map<std::string, int> sources;
  bool split_output;

  std::vector<Complex_buffer>                          accumulation_buffers;
  std::vector< std::vector<Complex_buffer> >           phase_centers;
  Complex_buffer_float                                 integration_buffer_float;
  std::vector< std::pair<size_t, size_t> >             baselines;
  int number_ffts_in_integration, number_ffts_in_sub_integration, current_fft, total_ffts;

  boost::shared_ptr<Data_writer>                       writer;

  Timer fft_timer;

  SFXC_FFT fft_f2t, fft_t2f;
  Complex_buffer temp_buffer;
  Real_buffer real_buffer;
  std::vector<FLOAT> window;

  // Needed for writing the progress messages
  int node_nr_;
  int current_integration;
  int next_sub_integration;
};

inline size_t Correlation_core::number_channels() {
  return correlation_parameters.number_channels;
}

inline size_t Correlation_core::fft_size() {
  return correlation_parameters.fft_size_correlation;
}

inline size_t Correlation_core::number_input_streams() {
  return input_buffers.size();
}

inline size_t Correlation_core::number_input_streams_in_use() {
  return correlation_parameters.station_streams.size();
}


#endif /*CORRELATION_CORE_H_*/
