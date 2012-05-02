#ifndef CORRELATION_CORE_PULSAR_H_
#define CORRELATION_CORE_PULSAR_H_

#include <fstream>
#include "correlation_core.h"

class Correlation_core_pulsar : public Correlation_core{
  typedef Pulsar_parameters::Pulsar Pulsar;
public:
  Correlation_core_pulsar();
  virtual ~Correlation_core_pulsar();
  virtual void do_task();
  void set_parameters(const Correlation_parameters &parameters, Pulsar &pulsar,
                              int node_nr);
protected:
  virtual void integration_initialise();
  void dedisperse_buffer();

  double get_phase();

  Pulsar_parameters::Polyco_params              *polyco;
  /// Temporary buffer to store un-dispersed data
  std::vector<Complex_buffer>                   dedispersion_buffer;
  // Offsets [in units of pulsar period] of frequency components relative to the reference frequency
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
  return start_phase + (polyco->coef[1]+60*polyco->ref_freq)*fft_duration*current_fft*1440/us_per_day;
}

#endif /*CORRELATION_CORE_H_*/
