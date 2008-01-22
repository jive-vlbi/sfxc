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
#include "utils.h"

class MPI_Transfer;

class Delay_table_akima
{
  friend class MPI_Transfer;
  
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

  //read the delay table, do some checks and
  //calculate coefficients for parabolic interpolation
  int open(const char *delayTableName);

  //calculate the delay for the delayType at time in microseconds
  double delay(int64_t time);

  /// A spline only interpolates one scan. 
  /// This functions preprocesses the spline for the next scan.
  bool initialise_next_scan();
  
  int64_t start_time_scan();
  int64_t stop_time_scan();
  
  bool initialised() const {
    return !times.empty();
  }
private:
  // Beginning of the scan and past the end pointer
  size_t begin_scan, end_scan;
  std::vector<double> times, delays;
  gsl_interp_accel *acc;
  gsl_spline *splineakima;
};


#endif // DELAY_TABLE_AKIMA_H
