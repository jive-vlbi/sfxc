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
#ifndef DELAYTABLE_H
#define DELAYTABLE_H

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
  double delay(int64_t time, int delayType) const;

  enum delayType {Cdel, Mdel, Rdel, Fdel};



private:
  std::vector<double> times, delays;
  gsl_interp_accel *acc;
  gsl_spline *splineakima;

  int cde, mde, rde; //switches which determine which columns of delay table are used
};


#endif // DELAYTABLE_H
