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

#define READ_SCAN_HEADER  0
#define FIND_TSTART  1
#define READ_NEW_SCAN 2

//*****************************************************************************
//function definitions
//*****************************************************************************

//default constructor, set default values
Uvw_model::Uvw_model()
    : scan_nr(0) {}

//destructor
Uvw_model::~Uvw_model() {}

void Uvw_model::operator=(const Uvw_model &other) {
  scan_nr = 0;
  sources = other.sources;
  scans = other.scans;
  times = other.times;
  u = other.u;
  v = other.v;
  w = other.w;
  splineakima_u.resize(0);
  splineakima_v.resize(0);
  splineakima_w.resize(0);
  acc_u.resize(0);
  acc_v.resize(0);
  acc_w.resize(0);
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
  if(!in.is_open())
    sfxc_abort((std::string("Could not open delay table ")+std::string(delayTableName)).c_str());
  int32_t header_size;

  // Read the header
  in.read(reinterpret_cast < char * > (&header_size), sizeof(int32_t));
  if (in.eof()) return 0;

  char header[header_size];
  in.read(reinterpret_cast < char * > (header), header_size*sizeof(char));
  if (in.eof()) return 0;

  double line[5], scan_start, scan_end;
  int32_t current_mjd;
  char current_source[81];
  int state = READ_SCAN_HEADER;
  bool done_reading = false;
  while((!done_reading) && (in.good())){
    switch(state){
    case READ_SCAN_HEADER:{
      if(in.read(current_source, sizeof(char) * 81)){
        in.read(reinterpret_cast < char * > (&current_mjd), sizeof(int32_t));
        in.read(reinterpret_cast < char * > (line), 5*sizeof(double));
        Time start_time_scan(current_mjd, line[0]);
        // strip whitespace from end of source string
        for(int i = 79; i >=0 ; i--){
          if(current_source[i] != ' '){
            current_source[i + 1] = 0;
            break;
          }
        }
        if(start_time_scan < tstart){
          state = FIND_TSTART;
        } else if (start_time_scan >= tstop){
          done_reading = true;
        }else {
          state = READ_NEW_SCAN;
        }
      }
      break;
    }
    case FIND_TSTART:{
      while (in.read(reinterpret_cast < char * > (line), 5*sizeof(double))) {
        SFXC_ASSERT(line[4] <= 0);
        Time time(current_mjd, line[0]);

        if(line[4] == 0){
          state = READ_SCAN_HEADER;
          break;
        }else if(time >= tstart){
          state = READ_NEW_SCAN;
          break;
        }
      }
      break;
    }
    case READ_NEW_SCAN:
      scan_start = line[0];
      Time start_time_scan(current_mjd, scan_start);
      // look up the current source in the list of sources and append it to the list if needed
      int source_index;
      for(source_index = 0; source_index < sources.size() ; source_index++){
        if(sources[source_index] == current_source)
          break;
      }
      if(source_index == sources.size())
        sources.push_back(current_source);

      // Create the new scan
      scans.resize(scans.size() + 1);
      Scan &scan = scans.back();
      scan.begin = start_time_scan;
      scan.source = source_index;
      scan.model_index = u.size();
      // Check if we are correlating an additional phase center to the current scan
      int n_scans = scans.size();
      if((n_scans > 1) && (scans[n_scans - 2].begin == start_time_scan)){
        // We found an additional phace center to the current scan
        scan.times = scans[n_scans - 2].times;
        times.resize(scan.times);
      }else{
        // We are staring a new scan
        scan.times = times.size();
      }
      // Read the data
      do {
        if (line[4] == 0) {
          if(times.size() == 1){
            // Instead of the first point of the desired scan, we got the
            // last point of the previous scan.  Get rid of it.
            scans.resize(0);
          }else{
            SFXC_ASSERT(scan_end > scan_start);
            scan.end.set_time(current_mjd, scan_end);
          }
          state = READ_SCAN_HEADER;
          break;
        } else {
          times.push_back(line[0] - scan_start);
          u.push_back(line[1]);
          v.push_back(line[2]);
          w.push_back(line[3]);
          scan_end = line[0];
        }
      } while(in.read(reinterpret_cast < char * > (line), 5*sizeof(double)));
      if((scans[n_scans - 2].begin == scan.begin) && (scans[n_scans - 2].end != scan.end))
        sfxc_abort("Premature ending of phase center\n");
    }
  }

  // Initialise
  scan_nr = 0;
  initialise_spline_for_next_scan();
  return 0;
}

void Uvw_model::initialise_spline_for_next_scan() {
  int n_sources_in_previous_scan = splineakima_u.size();
  if (scan_nr >= (scans.size() - n_sources_in_previous_scan))
    return;

  scan_nr += n_sources_in_previous_scan;

  for(int i = 0 ; i < splineakima_u.size() ; i++){
    gsl_spline_free(splineakima_u[i]);
    gsl_spline_free(splineakima_v[i]);
    gsl_spline_free(splineakima_w[i]);
    gsl_interp_accel_free(acc_u[i]);
    gsl_interp_accel_free(acc_v[i]);
    gsl_interp_accel_free(acc_w[i]);
  }

  // Determine the number of sources in current scan
  int n_sources = 1;
  while((scan_nr + n_sources < scans.size()) && 
        (scans[scan_nr + n_sources - 1].begin == scans[scan_nr + n_sources].begin))
    n_sources++;

  splineakima_u.resize(n_sources);
  splineakima_v.resize(n_sources);
  splineakima_w.resize(n_sources);
  acc_u.resize(n_sources);
  acc_v.resize(n_sources);
  acc_w.resize(n_sources);
  for(int i = 0; i < n_sources ; i++){
    Scan &scan = scans[scan_nr + i];
    // at least 4 sample points for a spline
    const Time one_sec(1000000);
    int n_pts = (int)((scan.end - scan.begin) / one_sec) + 1;
    SFXC_ASSERT(n_pts > 4);

    // Initialise the Akima spline
    acc_u[i] = gsl_interp_accel_alloc();
    acc_v[i] = gsl_interp_accel_alloc();
    acc_w[i] = gsl_interp_accel_alloc();
    splineakima_u[i] = gsl_spline_alloc(gsl_interp_akima, n_pts);
    splineakima_v[i] = gsl_spline_alloc(gsl_interp_akima, n_pts);
    splineakima_w[i] = gsl_spline_alloc(gsl_interp_akima, n_pts);

    gsl_spline_init(splineakima_u[i], &times[scan.times], &u[scan.model_index], n_pts);
    gsl_spline_init(splineakima_v[i], &times[scan.times], &v[scan.model_index], n_pts);
    gsl_spline_init(splineakima_w[i], &times[scan.times], &w[scan.model_index], n_pts);
  }
}

//calculates u,v, and w at time(microseconds)
void Uvw_model::get_uvw(int phase_center, Time time, double *u, double *v, double *w) {
  while (scans[scan_nr].end < time)
    initialise_spline_for_next_scan();

  SFXC_ASSERT(splineakima_u.size() > 0);
  SFXC_ASSERT((phase_center >= 0) && (phase_center < splineakima_u.size())) ;

  double sec = (time - scans[scan_nr].begin).get_time();
  *u = gsl_spline_eval(splineakima_u[phase_center], sec, acc_u[phase_center]);
  *v = gsl_spline_eval(splineakima_v[phase_center], sec, acc_v[phase_center]);
  *w = gsl_spline_eval(splineakima_w[phase_center], sec, acc_w[phase_center]);
//  std::cout << RANK_OF_NODE<< " : u=" << *u << ", v=" << *v << ", w=" << *w << " ; center = " << phase_center
//            << ", time = " << time << "\n";
  SFXC_ASSERT(*w < 0);
}

//calculates the delay for the delayType at time in microseconds
//get the next line from the delay table file
std::ofstream& Uvw_model::uvw_values(std::ofstream &output, Time starttime,
                                     Time stoptime, Time inttime) {
  starttime = scans.front().begin;
  stoptime = scans.back().end;

  Time time = starttime + inttime/2;
  double gsl_u, gsl_v, gsl_w;
  output.precision(14);
  std::cout.precision(16);
  while (time < stoptime) {
    while (scans[scan_nr].end < time) initialise_spline_for_next_scan();
    double sec = (time - scans[scan_nr].begin).get_time();
    gsl_u = gsl_spline_eval (splineakima_u[0], sec, acc_u[0]);
    gsl_v = gsl_spline_eval (splineakima_v[0], sec, acc_v[0]);
    gsl_w = gsl_spline_eval (splineakima_w[0], sec, acc_w[0]);
    double ttime  = time.get_time_usec();

//    output.write(reinterpret_cast < char * > (&ttime), sizeof(double));
//    output.write(reinterpret_cast < char * > (&gsl_u), sizeof(double));
//    output.write(reinterpret_cast < char * > (&gsl_v), sizeof(double));
//    output.write(reinterpret_cast < char * > (&gsl_w), sizeof(double));
    std::cout << (int64_t)ttime << " " <<  gsl_u << " " <<  gsl_v << " " <<  gsl_w << "\n";
    time += inttime;
  }
  return output;
}
