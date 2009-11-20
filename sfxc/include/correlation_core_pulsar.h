#ifndef CORRELATION_CORE_PULSAR_H_
#define CORRELATION_CORE_PULSAR_H_

#include <fstream>
#include "correlation_core.h"

//the number of elements in the dedispersion buffer
#define DEDISPERSION_BUFFER_LENGTH 1048576

class Correlation_core_pulsar : public Correlation_core{
public:
  Correlation_core_pulsar();
  virtual ~Correlation_core_pulsar();
  virtual void do_task();
  virtual void set_parameters(const Correlation_parameters &parameters,
                              int node_nr);
protected:
  virtual void integration_initialise();
  void dedisperse_buffer();

  double get_phase();

  Pulsar_parameters                             *pulsar_params;
  Pulsar_parameters::Polyco_params              *polyco;
  /// Temporary buffer to store un-dispersed data
  std::vector<Complex_buffer>                   dedispersion_buffer;
  // Time offsets of each frequency component relative to the reference frequency
  std::vector<double>                           offsets;
  std::vector<int>                              bins;
  /// The time bins are accumulated here
  std::vector< std::vector<Complex_buffer> >    accumulation_buffers;

  int nbins;
  struct{double begin;double end;} gate;
  double start_phase;   // Start phase of current slice [pulsar period].
  double fft_duration;  // The time one FFT window worth of data represents [us]
  int64_t us_per_day;
};

inline double Correlation_core_pulsar::get_phase(){
  // Because our time span is in the order of seconds we can increment the phase
  // with just the first order terms
/*  if(RANK_OF_NODE==13)
    std::cout << "coef=" << polyco->coef[1] << ", ref = " << polyco->ref_freq
              << ", duur="<< fft_duration << ", dt = " << (current_fft*1.+1./2)/us_per_day
              << "\n";*/
  return start_phase + (polyco->coef[1]+60*polyco->ref_freq)*fft_duration*(current_fft*1.+1./2)*1440/us_per_day;
}

#endif /*CORRELATION_CORE_H_*/
