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
#include "utils.h"
#include "correlator_time.h"

// GSL includes
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>


class MPI_Transfer;

class Uvw_model {
  friend class MPI_Transfer;

public:
  struct Scan{
    Time begin, end;
    int32_t source;
    int32_t times;
    int32_t model_index;
  };

  //constructor, set default values
  Uvw_model();

  //destructor
  ~Uvw_model();

  void operator=(const Uvw_model &other);
  bool operator==(const Uvw_model &other) const;

  //read the delay table, do some checks and
  //calculate coefficients for parabolic interpolation
  int open(const char *delayTableName);
  int open(const char *delayTableName, Time tstart, Time tstop);

  std::ofstream& uvw_values(std::ofstream &, Time starttime, Time stoptime,
                            Time inttime);

  //calculates u,v, and w at time(microseconds)
  void get_uvw(int phase_center, Time time, double *u, double *v, double *w);

  /// A spline only interpolates one scan.
  /// This functions preprocesses the spline for the next scan.
  void initialise_spline_for_next_scan();

  bool initialised() const {
    return !times.empty();
  }

private:
  // First entry of the next scan
  int scan_nr;
  std::vector<Scan> scans;
  std::vector<std::string> sources;
  std::vector<double> times, u, v, w;
  std::vector<gsl_interp_accel *> acc_u, acc_v, acc_w;
  std::vector<gsl_spline *> splineakima_u, splineakima_v, splineakima_w;
};


#endif // Uvw_model_H
