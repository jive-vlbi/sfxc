#ifndef BANDPASS_H
#define BANDPASS_H
#include <vector>
#include "correlator_time.h"

class bandpass{
public:
  bandpass();
  virtual ~bandpass(){}
  void apply_bandpass(const Time t, std::complex<FLOAT> *band, int station, double freq, char sideband, int pol_nr, bool do_conjg=false);
  void open_table(const char *filename, int nchan_, bool phase_only);
private:
  bool opened; // indicated if a bandpass table has been read yet
  int nstation, nif, npol, nchan, nchan_aips;
  int freq_nr; // Stores the index of the current frequency in the frequency table 
  std::vector<double> frequencies;
  std::vector<double> bandwidths;
  std::vector<std::vector<std::vector<std::complex<float> > > > bp_table; 
};
#endif
