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
  typedef struct {Time begin, end;} Interval;

public:
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

  //calculate the delay for the delayType at time in microseconds
  // delay is in seconds
  double delay(const Time &time);
  double rate(const Time &time);

  /// A spline only interpolates one scan.
  /// This functions preprocesses the spline for the next scan.
  bool initialise_next_scan();

  Time start_time_scan();
  Time stop_time_scan();

  bool initialised() const {
    return !times.empty();
  }
private:
  int begin_scan, scan_nr;
  std::vector<double> times, delays;
  std::vector<Interval> scans;
  gsl_interp_accel *acc;
  gsl_spline *splineakima;
};


std::ostream &operator<<(std::ostream &out,
                         const Delay_table_akima &delay_table);

#endif // DELAY_TABLE_AKIMA_H
