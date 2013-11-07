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
                              int node_nr);
protected:
  virtual void integration_initialise();
  void create_baselines(const Correlation_parameters &parameters);
  void integration_normalize();
  void integration_write_subints();
private:
  bandpass btable;
  bool is_open_;
  std::vector<Complex_buffer> accumulation_buffers;
  Complex_buffer              subint_conj_buffer;
  Complex_buffer              subint_buffer;
  Complex_buffer              autocor_buffer;
  Complex_buffer              autocor_conj_buffer;
  Complex_buffer              power_buffer;
};

#endif /*CORRELATION_CORE_H_*/
