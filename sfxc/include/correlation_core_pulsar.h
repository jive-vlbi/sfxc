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
  virtual void set_parameters(const Correlation_parameters &parameters,
                              int node_nr);
protected:
  virtual void integration_initialise();
  virtual void integration_step();
  virtual void integration_average();
  virtual void integration_write();
  void dedisperse_buffer();

  int n_bins();
  double get_phase();

  Pulsar_parameters                             *pulsar_params;
  Pulsar_parameters::Pulsar                     *cur_pulsar;
  Pulsar_parameters::Polyco_params              *polyco;
  /// Temporary buffer to store un-dispersed data
  std::vector<Complex_buffer>                   dedispersion_buffer;
  // Time offsets of each frequency component relative to the reference frequency
  std::vector<double>                           offsets;
  std::vector<int>                              bins;
  /// The time bins are accumulated here
  std::vector< std::vector<Complex_buffer> >    accumulation_buffers;
  Complex_buffer_float                          accumulation_buffers_float;

  double start_mjd;   // Start time of current slice [mjd].
  double start_phase; // Start phase of current slice [pulsar period].
  int fft_duration;   // The time one FFT window worth of data represents [us]
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
/*  double dph=(phase-floor(phase)-cur_pulsar->interval.start);
  double len=cur_pulsar->interval.stop-cur_pulsar->interval.start;
  if((dph<0)||(dph>len))
    return -1;

  SFXC_ASSERT((int)(dph*cur_pulsar->nbins/len)<cur_pulsar->nbins); //TODO remove me */
//   return (int)(dph*cur_pulsar->nbins/len);
}

inline int Correlation_core_pulsar::n_bins() {
  return cur_pulsar->nbins;
}

#endif /*CORRELATION_CORE_H_*/
