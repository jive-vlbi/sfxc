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

#include <pthread.h>
#include <types.h>
#include <vector>

// GSL includes
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#include "correlator_time.h"
#include "utils.h"

class MPI_Transfer;

class Delay_table_akima {
friend class Delay_table;
public:
  Delay_table_akima();
  Delay_table_akima(const Delay_table_akima &other);
  ~Delay_table_akima();
  void operator=(const Delay_table_akima &other);

  const std::string &get_source(int phase_center){
    return sources[phase_center];
  }
  // The number of phase centers in the current scan
  int n_phase_centers(){
    return splineakima.size();
  }
  // delay is in seconds
  double delay(const Time &time, int phase_center=0);
  double rate(const Time &time, int phase_center=0);
  double accel(const Time &time, int phase_center=0);
  double phase(const Time &time, int phase_center=0);
  double amplitude(const Time &time, int phase_center=0);
  Time scan_begin, interval_begin, interval_end; // FIXME make private again
private:
  void free_reference();
  void copy(const Delay_table_akima &other);
private:
  Time clock_epoch;
  double clock_offset, clock_rate;
  std::vector<std::string> sources;

  // GSL splining objects
  std::vector<gsl_interp_accel *> acc;
  std::vector<gsl_spline *> splineakima;
  std::vector<gsl_interp_accel *> acc_ph;
  std::vector<gsl_spline *> splineakima_ph;
  std::vector<gsl_interp_accel *> acc_amp;
  std::vector<gsl_spline *> splineakima_amp;
  // Mutexes
  pthread_mutex_t *mutex;
  int *ref_count;
};

class Delay_table {
  friend class MPI_Transfer;

public:
  struct Scan{
    Time begin, end;
    int32_t source;
    int32_t times;
    int32_t delays;
    int32_t phases;
    int32_t amplitudes;
  };

  // Constructor
  Delay_table();

  // Constructor
  Delay_table(const Delay_table &other);

  // Destructor
  ~Delay_table();

  // Assignment
  void operator=(const Delay_table &other);

  // Equality test
  bool operator==(const Delay_table &other) const;

  friend
  std::ostream &operator<<(std::ostream &out,
                           const Delay_table &delay_table);

  //read the delay table, do some checks and
  //calculate coefficients for parabolic interpolation
  void open(const char *delayTableName);
  void open(const char *delayTableName, const Time tstart, const Time tstop,
	    const std::string &source);

  //Set clock offset and rate
  void set_clock_offset(const double offset, const Time start, const double rate, const Time epoch);

  void add_scans(const Delay_table &other);

  // Get source at phase_center for current scan
  const std::string &get_source(int phase_center){
    return sources[scans[scan_nr + phase_center].source];
  }
  /// Move to the next scan
  bool initialise_next_scan();
  /// Create new akima spline valid for a time range
  Delay_table_akima create_akima_spline(const Time start, const Time duration);

  Time start_time_scan();
  Time stop_time_scan();

  bool initialised() const {
    return !scans.empty();
  }
private:
  int scan_nr;
  int clock_nr;
  int n_sources_in_current_scan;
  std::vector<Time> clock_starts;
  std::vector<double> clock_offsets;
  std::vector<Time> clock_epochs;
  std::vector<double> clock_rates;
  std::vector<Scan> scans;
  std::vector<std::string> sources;
  std::vector<double> times;
  std::vector<double> delays;
  std::vector<double> phases;
  std::vector<double> amplitudes;
  int tst;
};


std::ostream &operator<<(std::ostream &out,
                         const Delay_table &delay_table);

#endif // DELAY_TABLE_AKIMA_H
