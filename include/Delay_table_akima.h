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
#include <genPrms.h>

// GSL includes
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>

class MPI_Transfer;

class Delay_table_akima
{
  friend class MPI_Transfer;
  
public:
  //constructor, set default values 
  Delay_table_akima();

  //destructor
  ~Delay_table_akima();
    
  bool operator==(const Delay_table_akima &other) const;

  void set_cmr(GenP GenPrms);

  //read the delay table, do some checks and
  //calculate coefficients for parabolic interpolation
  int open(char *delayTableName);

  //calculate the delay for the delayType at time in microseconds
  double delay(int64_t time, int delayType);

  enum delayType {Cdel, Mdel, Rdel, Fdel};

  /// A spline only interpolates one scan. 
  /// This functions preprocesses the spline for the next scan.
  void initialise_spline_for_next_scan();
private:
  // Last entry of the previous scan
  size_t end_scan;
  std::vector<double> times, delays;
  gsl_interp_accel *acc;
  gsl_spline *splineakima;

  int32_t cde, mde, rde; //switches which determine which columns of delay table are used
};


#endif // DELAY_TABLE_AKIMA_H
