#ifndef CORRELATION_CORE_PHASED_H_
#define CORRELATION_CORE_PHASED_H_

#include <fstream>
#include "correlation_core.h"

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
  virtual void integration_initialise();
  void integration_step(std::vector<Complex_buffer> &integration_buffer, int buf_idx);

  void create_baselines(const Correlation_parameters &parameters);
};

#endif /*CORRELATION_CORE_H_*/
