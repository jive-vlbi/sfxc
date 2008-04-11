/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Ruud Oerlemans  <Oerlemans@JIVE.nl>, 2007
 *            Nico Kruithof   <Kruithof@JIVE.nl>, 2007
 *            Huseyin Ozdemir <Ozdemir@JIVE.nl>, 2007
 *
 * $Id: Uvw_model.h 301 2007-08-29 08:04:16Z kruithof $
 *
 * Class definitions for delay table
 */
#ifndef Uvw_model_H
#define Uvw_model_H

#include <types.h>
#include <vector>

// GSL includes
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>

#include "utils.h"


class MPI_Transfer;

class Uvw_model {
  friend class MPI_Transfer;

public:
  //constructor, set default values
  Uvw_model();

  //destructor
  ~Uvw_model();

  bool operator==(const Uvw_model &other) const;

  //read the delay table, do some checks and
  //calculate coefficients for parabolic interpolation
  int open(char *delayTableName);

  std::ofstream& uvw_values(std::ofstream &, int64_t starttime, int64_t stoptime,
                            double inttime);

  /// A spline only interpolates one scan.
  /// This functions preprocesses the spline for the next scan.
  void initialise_spline_for_next_scan();
private:
  // Last entry of the previous scan
  size_t end_scan;
  std::vector<double> times, u, v, w;
  gsl_interp_accel *acc;
  gsl_spline *splineakima_u;
  gsl_spline *splineakima_v;
  gsl_spline *splineakima_w;

};


#endif // Uvw_model_H
