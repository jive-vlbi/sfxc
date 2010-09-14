/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Ruud Oerlemans  <Oerlemans@JIVE.nl>, 2007
 *            Nico Kruithof   <Kruithof@JIVE.nl>, 2007
 *            Huseyin Ozdemir <Ozdemir@JIVE.nl>, 2007
 *
 * $Id$
 *
 * Class function definitions for station specific data
 */

//the class definitions and function definitions
#include "utils.h"
#include "uvw_model.h"

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//includes for system calls
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

//*****************************************************************************
//function definitions
//*****************************************************************************

//default constructor, set default values
Uvw_model::Uvw_model()
    : begin_scan(0), scan_nr(-1), acc_u(NULL), acc_v(NULL), acc_w(NULL), 
      splineakima_u(NULL), splineakima_v(NULL), splineakima_w(NULL) {}

//destructor
Uvw_model::~Uvw_model() {}

void Uvw_model::operator=(const Uvw_model &other) {
  Uvw_model();

  times = other.times;
  u = other.u;
  v = other.v;
  w = other.w;
  scans = other.scans;
  initialise_spline_for_next_scan();
}


bool Uvw_model::operator==(const Uvw_model &other) const {
  return true;
}

//read the delay table, do some checks and
//calculate coefficients for parabolic interpolation
int Uvw_model::open(const char *delayTableName) {
  const Time start, stop = Time::max_time();
  return open(delayTableName, start, stop);
}

int Uvw_model::open(const char *delayTableName, Time tstart, Time tstop) {
  std::ifstream in(delayTableName);
  double line[5];
  int32_t hsize;

  in.read(reinterpret_cast < char * > (&hsize), sizeof(int32_t));
  char station[hsize];
  in.read(reinterpret_cast < char * > (station), hsize*sizeof(char));

  // Read up to tstart
  Time time;
  int32_t current_mjd;
  double scan_start, scan_end;
  // The first line of each scan is the mjd of the scan
  in.read(reinterpret_cast < char * > (&current_mjd), sizeof(int32_t));
  while (in.read(reinterpret_cast < char * > (line), 5*sizeof(double))) {
    time.set_time(current_mjd, line[0]);
    if (line[4] == 0)
      in.read(reinterpret_cast < char * > (&current_mjd), sizeof(int32_t));
    else if (time >= tstart) {
      scan_start = line[0];
      times.push_back(0.);
      u.push_back(line[1]);
      v.push_back(line[2]);
      w.push_back(line[3]);
      scans.resize(1);
      scans[0].begin = time;
      break;
    }
  }

  // Read the rest of the data
  while (in.read(reinterpret_cast < char * > (line), 5*sizeof(double))) {
    SFXC_ASSERT(line[4] <= 0);
    time.set_time(current_mjd, line[0]);
    if (line[4] == 0) {
      if(times.size() == 1){
        // Instead of the first point of the desired scan, we got the
        // last point of the previous scan.  Get rid of it.
        times.resize(0);
        u.resize(0);
        v.resize(0);
        w.resize(0);
        scans.resize(0);
      }else{
       Interval &scan = scans.back();
       scan.end.set_time(current_mjd, scan_end);
       scan_start = -1;
      }
      in.read(reinterpret_cast < char * > (&current_mjd), sizeof(int32_t));
    } else {
      if(scan_start < 0){
        scan_start = line[0];
        scans.push_back((Interval){time, Time()});
      }
      times.push_back(line[0] - scan_start);
      u.push_back(line[1]);
      v.push_back(line[2]);
      w.push_back(line[3]);
      scan_end = line[0];
    }
    if (time >= tstop){
      Interval &scan = scans.back();
      scan.end.set_time(current_mjd, scan_end);
      break;
    }
  }

  begin_scan = 0;
  scan_nr = -1;
  initialise_spline_for_next_scan();
  return 0;
}

void Uvw_model::initialise_spline_for_next_scan() {
  if (begin_scan >= times.size())
    return;

  scan_nr++;

  if (splineakima_u != NULL) {
    gsl_spline_free(splineakima_u);
    gsl_interp_accel_free(acc_u);
  }
  if (splineakima_v != NULL) {
    gsl_spline_free(splineakima_v);
    gsl_interp_accel_free(acc_v);
  }
  if (splineakima_w != NULL) {
    gsl_spline_free(splineakima_w);
    gsl_interp_accel_free(acc_w);
  }

  // Initialise the Akima spline
  acc_u = gsl_interp_accel_alloc();
  acc_v = gsl_interp_accel_alloc();
  acc_w = gsl_interp_accel_alloc();
  int n_pts = (int)(scans[scan_nr].end - scans[scan_nr].begin).get_time() + 1;
  // at least 4 sample points for a spline
  SFXC_ASSERT(n_pts > 4);

  // End scan now points to the beginning of the next scan and
  // the next scan has n_pts data points
  splineakima_u = gsl_spline_alloc(gsl_interp_akima, n_pts);
  splineakima_v = gsl_spline_alloc(gsl_interp_akima, n_pts);
  splineakima_w = gsl_spline_alloc(gsl_interp_akima, n_pts);

  gsl_spline_init(splineakima_u,
                  &times[begin_scan],
                  &u[begin_scan],
                  n_pts);
  gsl_spline_init(splineakima_v,
                  &times[begin_scan],
                  &v[begin_scan],
                  n_pts);
  gsl_spline_init(splineakima_w,
                  &times[begin_scan],
                  &w[begin_scan],
                  n_pts);
  begin_scan += n_pts;
}

//calculates u,v, and w at time(microseconds)
void Uvw_model::get_uvw(Time time, double *u, double *v, double *w) {
  if (times.empty()) {
    DEBUG_MSG("times.empty()");
    SFXC_ASSERT(!times.empty());
  }

  while (scans[scan_nr].end < time) {
    initialise_spline_for_next_scan();
  }

  SFXC_ASSERT(splineakima_u != NULL);
  SFXC_ASSERT(splineakima_v != NULL);
  SFXC_ASSERT(splineakima_w != NULL);
  double sec = (time - scans[scan_nr].begin).get_time();

  *u = gsl_spline_eval (splineakima_u, sec, acc_u);
  *v = gsl_spline_eval (splineakima_v, sec, acc_v);
  *w = gsl_spline_eval (splineakima_w, sec, acc_w);
}

//calculates the delay for the delayType at time in microseconds
//get the next line from the delay table file
std::ofstream& Uvw_model::uvw_values(std::ofstream &output, Time starttime,
                                     Time stoptime, Time inttime) {
  Time time = starttime + inttime/2;
  double gsl_u, gsl_v, gsl_w;
  output.precision(14);
  while (time < stoptime) {
    while (scans[scan_nr].end < time) initialise_spline_for_next_scan();
    double sec = (time - scans[scan_nr].begin).get_time();
    gsl_u = gsl_spline_eval (splineakima_u, sec, acc_u);
    gsl_v = gsl_spline_eval (splineakima_v, sec, acc_v);
    gsl_w = gsl_spline_eval (splineakima_w, sec, acc_w);
    double ttime  = time.get_time();

    output.write(reinterpret_cast < char * > (&ttime), sizeof(double));
    output.write(reinterpret_cast < char * > (&gsl_u), sizeof(double));
    output.write(reinterpret_cast < char * > (&gsl_v), sizeof(double));
    output.write(reinterpret_cast < char * > (&gsl_w), sizeof(double));
    time += inttime;
  }
  return output;
}
