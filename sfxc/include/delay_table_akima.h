/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Ruud Oerlemans  <Oerlemans@JIVE.nl>, 2007
 *            Nico Kruithof   <Kruithof@JIVE.nl>, 2007
 *            Huseyin Ozdemir <Ozdemir@JIVE.nl>, 2007
 *
 * $Id$
 *
 * Class definitions for delay table
 */
#ifndef DELAY_TABLE_AKIMA_H
#define DELAY_TABLE_AKIMA_H

#include <types.h>
#include <vector>

// GSL includes
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#include "correlator_time.h"
#include "utils.h"

class MPI_Transfer;

class Delay_table_akima {
  friend class MPI_Transfer;

public:
  struct Scan{
    Time begin, end;
    int32_t source;
    int32_t times;
    int32_t delays;
    int32_t phases;
  };

  // Constructor
  Delay_table_akima();

  // Constructor
  Delay_table_akima(const Delay_table_akima &other);

  // Destructor
  ~Delay_table_akima();

  // Assignment
  void operator=(const Delay_table_akima &other);

  // Equality test
  bool operator==(const Delay_table_akima &other) const;

  friend
  std::ostream &operator<<(std::ostream &out,
                           const Delay_table_akima &delay_table);

  //read the delay table, do some checks and
  //calculate coefficients for parabolic interpolation
  void open(const char *delayTableName);
  void open(const char *delayTableName, const Time tstart, const Time tstop);
  //Set clock offset and rate
  void set_clock_offset(const double offset, const double rate, const Time epoch);

  //calculate the delay for the delayType at time in microseconds
  // delay is in seconds
  double delay(const Time &time, int phase_center=0);
  double rate(const Time &time, int phase_center=0);
  double phase(const Time &time, int phase_center=0);

  // Moves the delay table to the requested scan
  bool goto_scan(const Time &time);
  // The number of phase centers in the current scan
  int n_phase_centers(){
    return splineakima.size();
  }
  // Get the array of sources
  const std::vector<std::string> &get_source_list(){
    return sources;
  }
  // Get source at phase_center for current scan
  const std::string &get_source(int phase_center){
    return sources[scans[scan_nr + phase_center].source];
  }
  // Get index of source at phase_center for current scan
  int get_source_index(int phase_center){
    return scans[scan_nr + phase_center].source;
  }
  /// A spline only interpolates one scan.
  /// This functions preprocesses the spline for the next scan.
  bool initialise_next_scan();

  Time start_time_scan();
  Time stop_time_scan();

  bool initialised() const {
    return !scans.empty();
  }
private:
  int scan_nr;
  Time clock_epoch;
  double clock_offset, clock_rate;
  std::vector<Scan> scans;
  std::vector<std::string> sources;
  std::vector<double> times;
  std::vector<double> delays;
  std::vector<double> phases;
  std::vector<gsl_interp_accel *> acc;
  std::vector<gsl_spline *> splineakima;
  std::vector<gsl_interp_accel *> acc_ph;
  std::vector<gsl_spline *> splineakima_ph;
};


std::ostream &operator<<(std::ostream &out,
                         const Delay_table_akima &delay_table);

#endif // DELAY_TABLE_AKIMA_H
