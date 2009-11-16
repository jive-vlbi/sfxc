#ifndef CORRELATION_CORE_H_
#define CORRELATION_CORE_H_

#include "tasklet/tasklet.h"
#include "delay_correction_base.h"
#include "control_parameters.h"
#include "data_writer.h"
#include "uvw_model.h"

#include "timer.h"
#include <fstream>

class Correlation_core : public Tasklet {
friend class Correlation_core_pulsar;
public:
  // Simple iterator class TODO 
  template<class T, typename U>
  class simple_it{
  public:
    simple_it(T *data_, int size_):data(data_),index(0) {}
    simple_it():data(NULL), index(0), size(-1) {}
    void set(T *data_, int size_){
      data=data_;
      size=size_;
      index=0;
    }
    inline T *operator->(){
      return &data[index];
    }
    inline T &operator*(){
      return data[index];
    }
    inline U &operator[](int i){
      return data[index][i];
    }
    inline void operator++(int){
      index++;
    }
    inline bool valid(){
      return index<size;
    }
    // TODO make private again
    int size;
    int index;
  private:
    T *data;
  };
  typedef Delay_correction_base::Output_buffer_element       Input_buffer_element;
  typedef Delay_correction_base::Output_buffer               Input_buffer;
  typedef Delay_correction_base::Output_buffer_ptr           Input_buffer_ptr;
  typedef Delay_correction_base::Output_data                 Input_data;

  typedef Memory_pool_vector_element<FLOAT>                Float_buffer;
  typedef Memory_pool_vector_element<std::complex<FLOAT> > Complex_buffer;
  typedef Memory_pool_vector_element<std::complex<float> > Complex_buffer_float;

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

  void connect_to(size_t stream, Input_buffer_ptr buffer);

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

protected:
  virtual void integration_initialise();
  void integration_step(std::vector<Complex_buffer> &integration_buffer);
  void integration_normalize(std::vector<Complex_buffer> &integration_buffer);
  void integration_write(std::vector<Complex_buffer> &integration_buffer);

  void auto_correlate_baseline(std::complex<FLOAT> in[],
                               std::complex<FLOAT> out[]);

  void correlate_baseline(std::complex<FLOAT> in1[],
                          std::complex<FLOAT> in2[],
                          std::complex<FLOAT> out[]);


  size_t size_of_fft();
  size_t n_channels();
  size_t n_stations();

  size_t number_input_streams_in_use();

protected:
  std::vector<Input_buffer_ptr>  input_buffers;
  // Used in integration_step(), avoids contruction and destroying the vectors
  std::vector< simple_it< Input_data, std::complex<FLOAT> > >    input_elements;

  Correlation_parameters                               correlation_parameters;

  std::vector<Complex_buffer>                          accumulation_buffers;
  Complex_buffer_float                                 integration_buffer_float;
  std::vector< std::pair<size_t, size_t> >             baselines;
  int number_ffts_in_integration, current_fft, total_ffts;

  boost::shared_ptr<Data_writer>                       writer;

  Timer fft_timer;

  // Needed for writing the progress messages
  int node_nr_;
  int current_integration;

  bool check_input_elements;

#ifdef SFXC_WRITE_STATS
  // For plotting statistics on the height of the fringe and the phase
  std::ofstream                                        stats_out;
  Complex_buffer                                       backward_buffer;
  FFTW_PLAN                                            backward_plan_;
#endif // SFXC_WRITE_STATS

};

inline size_t Correlation_core::n_channels() {
  return correlation_parameters.number_channels;
}

inline size_t Correlation_core::size_of_fft() {
  return n_channels()*2;
}

inline size_t Correlation_core::n_stations() {
  return correlation_parameters.station_streams.size();
}

inline size_t Correlation_core::number_input_streams_in_use() {
  return correlation_parameters.station_streams.size();
}


#endif /*CORRELATION_CORE_H_*/
