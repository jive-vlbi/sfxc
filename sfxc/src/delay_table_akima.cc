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

#include "delay_table_akima.h"
#include "utils.h"

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

// Default constructor
Delay_table_akima::Delay_table_akima()
    : acc(NULL), splineakima(NULL), begin_scan(0), scan_nr(-1) {}

// Copy constructor
Delay_table_akima::Delay_table_akima(const Delay_table_akima &other)
    : acc(NULL), splineakima(NULL), begin_scan(0), scan_nr(-1) {
  SFXC_ASSERT(splineakima == NULL);
  times = other.times;
  delays = other.delays;
  scans = other.scans;
  initialise_next_scan();
}

// Destructor
Delay_table_akima::~Delay_table_akima() {}

void Delay_table_akima::operator=(const Delay_table_akima &other) {
  begin_scan = 0;
  scan_nr = -1;
  times = other.times;
  delays = other.delays;
  scans = other.scans;
  initialise_next_scan();
}

bool Delay_table_akima::operator==(const Delay_table_akima &other) const {
  if (times != other.times) return false;
  if (delays != other.delays) return false;
  return true;
}

//read the delay table, do some checks and
//calculate coefficients for parabolic interpolation
void Delay_table_akima::open(const char *delayTableName) {
  const Time start, stop = Time::max_time();
  open(delayTableName, start, stop);
}

void Delay_table_akima::open(const char *delayTableName, const Time tstart, const Time tstop) {
  std::ifstream in(delayTableName);
  if(!in.is_open())
    sfxc_abort((std::string("Could not open delay table ")+std::string(delayTableName)).c_str());
  int32_t header_size;

  // Read the header
  in.read(reinterpret_cast < char * > (&header_size), sizeof(int32_t));
  if (in.eof()) return;

  char header[header_size];
  in.read(reinterpret_cast < char * > (header), header_size*sizeof(char));
  if (in.eof()) return;

  // Read up to tstart
  Time time;
  double line[5], scan_start, scan_end;
  int32_t current_mjd;
  // The first line of each scan is the mjd of the scan
  in.read(reinterpret_cast < char * > (&current_mjd), sizeof(int32_t));
  while (in.read(reinterpret_cast < char * > (line), 5*sizeof(double))) {
    SFXC_ASSERT(line[4] <= 0);
    time.set_time(current_mjd, line[0]);
    if (line[4] == 0)
      in.read(reinterpret_cast < char * > (&current_mjd), sizeof(int32_t));
    else if (time >= tstart) {
      scan_start = line[0];
      times.push_back(0.);
      delays.push_back(line[4]);
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
        delays.resize(0);
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
      delays.push_back(line[4]);
      scan_end = line[0];
    }
    if (time >= tstop){
      Interval &scan = scans.back();
      scan.end.set_time(current_mjd, scan_end);
      break;
    }
  }

  // Initialise
  begin_scan = 0;
  scan_nr = -1;
  initialise_next_scan();
}

bool Delay_table_akima::initialise_next_scan() {
  if (begin_scan >= times.size())
    return false;

  scan_nr++;

  if (splineakima != NULL) {
    gsl_spline_free(splineakima);
    gsl_interp_accel_free(acc);
  }

  // Initialise the Akima spline
  acc = gsl_interp_accel_alloc();
  int n_pts = (int)(scans[scan_nr].end - scans[scan_nr].begin).get_time() + 1;

  // at least 4 sample points for a spline
  SFXC_ASSERT(n_pts > 4);

  // End scan now points to the beginning of the next scan and
  // the next scan has n_pts data points
  splineakima = gsl_spline_alloc(gsl_interp_akima, n_pts);

  gsl_spline_init(splineakima,
                  &times[begin_scan],
                  &delays[begin_scan],
                  n_pts);

  begin_scan += n_pts;
  return true;
}


//calculates the delay for the delayType at time
//get the next line from the delay table file
double Delay_table_akima::delay(const Time &time) {
  SFXC_ASSERT(!times.empty());

  while (scans[scan_nr].end < time)
    initialise_next_scan();

  SFXC_ASSERT(splineakima != NULL);
  double sec = (time - scans[scan_nr].begin).get_time();
  double result = gsl_spline_eval (splineakima, sec, acc);
  SFXC_ASSERT(result < 0);
  return result;
}

double Delay_table_akima::rate(const Time &time) {
  SFXC_ASSERT(!times.empty());

  while (scans[scan_nr].end < time)
    initialise_next_scan();

  SFXC_ASSERT(splineakima != NULL);
  double sec = (time - scans[scan_nr].begin).get_time();
  double result = gsl_spline_eval_deriv (splineakima, sec, acc);
  return result;
}

Time Delay_table_akima::start_time_scan() {
  SFXC_ASSERT(scan_nr < scans.size());
  return scans[scan_nr].begin;
}
Time Delay_table_akima::stop_time_scan() {
  SFXC_ASSERT(scan_nr < scans.size());
  return scans[scan_nr].end;
}

std::ostream &
operator<<(std::ostream &out, const Delay_table_akima &delay_table) {
  SFXC_ASSERT(delay_table.times.size() == delay_table.delays.size());
  for (size_t i=0; i<delay_table.times.size(); i++) {
    out << delay_table.times[i] << " \t" << delay_table.delays[i] << "\n";
  }
  return out;
}
