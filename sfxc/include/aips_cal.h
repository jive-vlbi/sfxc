#ifndef AIPS_CAL_H
#define AIPS_CAL_H
#include <vector>
#include "utils.h"
#include "correlator_time.h"

class aips_cal{
public:
  aips_cal();
  virtual ~aips_cal(){}
  void compute_calibration(const Time t, int station, int freq_nr, int pol_nr, char sideband);
  void apply_calibration(const Time t, std::complex<FLOAT> *band, int station,
                         double freq, char sideband, int pol_nr, bool do_conjg=false);
  void open_table(const std::string &filename, int nchan_, Time recompute_delay_=10000);
  bool is_open() {return opened;}
public:
  struct cl_data{
    std::vector<double> delays, rates, weights, disp_delays;
    std::vector<std::complex<double> > gains;
  };
private:
  void init();
  int find_next_row(int station, int pol_nr, int if_nr);
private:
  struct Table_data{ 
    Time time; 
    int freq_nr, pol_nr;
    std::vector<std::complex<FLOAT> > table;
    std::vector<std::complex<FLOAT> > table_conjg;
  }; 
private:
  bool opened; // indicated if the CL table has been read yet
  // The time between two recomputations of the calibration tables
  Time recompute_time;
  // The calibration tables 
  std::vector<Table_data> calib_tables;
  std::vector<std::vector<int> > current_row, next_row;
  int freq_nr; 
  int start_mjd, nstation, nif, npol;
  int nchan; // The number of channels used in the current correlation
  int nchan_aips; // The number of channels used to create the current CL table
  std::vector<double> frequencies;
  std::vector<double> bandwidths;
  std::vector<Time> times, time_interval;
  std::vector<cl_data> cl_table;
};
#endif
