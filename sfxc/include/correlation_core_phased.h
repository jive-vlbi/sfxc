#ifndef CORRELATION_CORE_PHASED_H_
#define CORRELATION_CORE_PHASED_H_

#include "correlation_core.h"
#include "bandpass.h"

class Correlation_core_phased : public Correlation_core{
public:
  Correlation_core_phased();
  virtual ~Correlation_core_phased();
  virtual void do_task();
  virtual void set_parameters(const Correlation_parameters &parameters,
                              std::vector<Delay_table_akima> &delays,
                              std::vector<std::vector<double> > &uvw,
                              int node_nr);
protected:
  void integration_initialise();
  void integration_step(std::vector<Complex_buffer> &integration_buffer, 
                        int first, int last, int stride);
  void sub_integration();
  void integration_write_subints(std::vector<Complex_buffer> &integration_buffer);
  bool use_autocorrelations;
};

#endif /*CORRELATION_CORE_H_*/
