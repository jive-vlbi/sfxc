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

#define READ_SCAN_HEADER  0
#define FIND_TSTART  1
#define START_NEW_SCAN 2
#define READ_PHASE_CENTER 3


//*****************************************************************************
//function definitions
//*****************************************************************************

// Default constructor
Delay_table_akima::Delay_table_akima()
  : scan_nr(0), clock_nr(0) {
}

// Copy constructor
Delay_table_akima::Delay_table_akima(const Delay_table_akima &other)
  : scan_nr(0), clock_nr(0) {
  clock_starts = other.clock_starts;
  clock_offsets = other.clock_offsets;
  clock_rates = other.clock_rates;
  clock_epochs = other.clock_epochs;
  sources = other.sources;
  scans = other.scans;
  times = other.times;
  delays = other.delays;
  phases = other.phases;
  amplitudes = other.amplitudes;
  initialise_next_scan();
}

// Destructor
Delay_table_akima::~Delay_table_akima() {}

void Delay_table_akima::operator=(const Delay_table_akima &other) {
  scan_nr = 0;
  clock_starts = other.clock_starts;
  clock_offsets = other.clock_offsets;
  clock_rates = other.clock_rates;
  clock_epochs = other.clock_epochs;
  sources = other.sources;
  scans = other.scans;
  times = other.times;
  delays = other.delays;
  phases = other.phases;
  amplitudes = other.amplitudes;
  for(int i = 0 ; i < splineakima.size() ; i++){
    gsl_spline_free(splineakima[i]);
    gsl_spline_free(splineakima_ph[i]);
    gsl_spline_free(splineakima_amp[i]);
    gsl_interp_accel_free(acc[i]);
    gsl_interp_accel_free(acc_ph[i]);
    gsl_interp_accel_free(acc_amp[i]);
  }
  splineakima.resize(0);
  acc.resize(0);
  initialise_next_scan();
}

bool Delay_table_akima::operator==(const Delay_table_akima &other) const {
  if(scans.size() != other.scans.size()) return false;
  for(int i = 0 ; i < delays.size() ; i++){
    if (delays[i] != other.delays[i]) return false;
  }
  return true;
}

// Set clock offset and rate
void Delay_table_akima::set_clock_offset(const double offset, const Time start, const double rate, const Time epoch){
  clock_starts.push_back(start);
  clock_offsets.push_back(offset);
  clock_rates.push_back(rate);
  clock_epochs.push_back(epoch);
}

//read the delay table, do some checks and
//calculate coefficients for parabolic interpolation
void Delay_table_akima::open(const char *delayTableName) {
  const Time start, stop = Time::max_time();
  open(delayTableName, start, stop);
}

void Delay_table_akima::open(const char *delayTableName, const Time tstart, const Time tstop){
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

  double line[7], scan_start, scan_end;
  int32_t current_mjd;
  char current_source[81];

  int correlation_scan = 0;
  int state = READ_SCAN_HEADER;
  bool done_reading = false;
  while((!done_reading) && (in.good())){
    switch(state){
    case READ_SCAN_HEADER:{
      if(in.read(current_source, sizeof(char) * 81)){
        in.read(reinterpret_cast < char * > (&current_mjd), sizeof(int32_t));
        in.read(reinterpret_cast < char * > (line), 7*sizeof(double));
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
          state = START_NEW_SCAN;
        }
      }
      break;
    }
    case FIND_TSTART:{
      while (in.read(reinterpret_cast < char * > (line), 7*sizeof(double))) {

        Time time(current_mjd, line[0]);

        if(line[4] == 0){
          state = READ_SCAN_HEADER;
          break;
        }else if(time >= tstart){
          state = START_NEW_SCAN;
          break;
        }
      }
      break;
    }
    case START_NEW_SCAN:{
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
      scan.delays = delays.size();
      scan.phases = phases.size();
      scan.amplitudes = amplitudes.size();
      // Check if we are correlating an additional phase center to the current scan
      int n_scans = scans.size();
      if((n_scans > 1) && (scans[n_scans - 2].begin == start_time_scan)){
        // We found an additional phace center to the current scan, overwrite previous times
        times.resize(scans[n_scans - 2].times);
      }
      scan.times = times.size();
      state = READ_PHASE_CENTER;
      break;
    }
    case READ_PHASE_CENTER:{
      Scan &scan = scans.back();
      // Read the data
      do {
        if (line[4] == 0) {
          if(times.size() == 1){
            // Instead of the first point of the desired scan, we got the
            // last point of the previous scan.  Get rid of it.
            scans.resize(0);
            times.resize(0);
          }else{
            SFXC_ASSERT(scan_end > scan_start);
            scan.end.set_time(current_mjd, scan_end);
          }
          state = READ_SCAN_HEADER;
          break;
        } else {
          times.push_back(line[0] - scan_start);
          delays.push_back(line[4]);
          phases.push_back(line[5]);
          amplitudes.push_back(line[6]);
          scan_end = line[0];
        }
      } while(in.read(reinterpret_cast < char * > (line), 7*sizeof(double)));
      correlation_scan = scans.size() - 1;
      break;
    }}
  }
  // Initialise
  scan_nr = 0;
  initialise_next_scan();
}

void
Delay_table_akima::add_scans(const Delay_table_akima &other)
{
  if (scans.empty()) {
    *this = other;
    return;
  }

  int prev_scans_size = scans.size();
  scans.insert(scans.end(), other.scans.begin(), other.scans.end());
  for (int i = prev_scans_size; i < scans.size(); i++) {
    scans[i].source += sources.size();
    scans[i].times += times.size();
    scans[i].delays += delays.size();
    scans[i].phases += phases.size();
    scans[i].amplitudes += amplitudes.size();
  }

  sources.insert(sources.end(), other.sources.begin(), other.sources.end());
  times.insert(times.end(), other.times.begin(), other.times.end());
  delays.insert(delays.end(), other.delays.begin(), other.delays.end());
  phases.insert(phases.end(), other.phases.begin(), other.phases.end());
  amplitudes.insert(amplitudes.end(), other.amplitudes.begin(),
		    other.amplitudes.end());
  clock_starts.insert(clock_starts.end(), other.clock_starts.begin(),
		      other.clock_starts.end());
  clock_offsets.insert(clock_offsets.end(), other.clock_offsets.begin(),
		      other.clock_offsets.end());
  clock_epochs.insert(clock_epochs.end(), other.clock_epochs.begin(),
		      other.clock_epochs.end());
  clock_rates.insert(clock_rates.end(), other.clock_rates.begin(),
		      other.clock_rates.end());
}

bool Delay_table_akima::initialise_next_scan() {
  int n_sources_in_previous_scan = splineakima.size();
  if (scan_nr >= (scans.size() - n_sources_in_previous_scan))
    return false;

  scan_nr += n_sources_in_previous_scan;

  for(int i = 0 ; i < splineakima.size() ; i++){
    gsl_spline_free(splineakima[i]);
    gsl_spline_free(splineakima_ph[i]);
    gsl_spline_free(splineakima_amp[i]);
    gsl_interp_accel_free(acc[i]);
    gsl_interp_accel_free(acc_ph[i]);
    gsl_interp_accel_free(acc_amp[i]);
  }

  // Determine the number of sources in current scan
  int n_sources = 1;
  while((scan_nr + n_sources < scans.size()) && 
        (scans[scan_nr + n_sources - 1].begin == scans[scan_nr + n_sources].begin))
    n_sources++;

  splineakima.resize(n_sources);
  splineakima_ph.resize(n_sources);
  splineakima_amp.resize(n_sources);
  acc.resize(n_sources);
  acc_ph.resize(n_sources);
  acc_amp.resize(n_sources);
  for(int i = 0; i < n_sources ; i++){
    Scan &scan = scans[scan_nr + i];
    // at least 4 sample points for a spline
    const Time one_sec(1000000);
    int n_pts = (scan.end - scan.begin) / one_sec + 1;
    SFXC_ASSERT(n_pts > 4);

    // Initialise the Akima spline
    acc[i] = gsl_interp_accel_alloc();
    acc_ph[i] = gsl_interp_accel_alloc();
    acc_amp[i] = gsl_interp_accel_alloc();
    splineakima[i] = gsl_spline_alloc(gsl_interp_akima, n_pts);
    splineakima_ph[i] = gsl_spline_alloc(gsl_interp_akima, n_pts);
    splineakima_amp[i] = gsl_spline_alloc(gsl_interp_akima, n_pts);
    gsl_spline_init(splineakima[i], &times[scan.times], &delays[scan.delays], n_pts);
    gsl_spline_init(splineakima_ph[i], &times[scan.times], &phases[scan.phases], n_pts);
    gsl_spline_init(splineakima_amp[i], &times[scan.times], &amplitudes[scan.amplitudes], n_pts);
  }

  clock_nr = clock_starts.size() - 1;
  while (clock_nr > 0 && clock_starts[clock_nr] > scans[scan_nr].begin)
    clock_nr--;

  return true;
}

//calculates the delay for the delayType at time
//get the next line from the delay table file
double Delay_table_akima::delay(const Time &time, int phase_center) {
  while (scans[scan_nr].end < time){
    if (!initialise_next_scan()) break;
  }
  SFXC_ASSERT(time >= scans[scan_nr].begin);
  SFXC_ASSERT(time <= scans[scan_nr].end);
  SFXC_ASSERT(splineakima.size() > 0);
  double sec = (time - scans[scan_nr].begin).get_time();
  double result = gsl_spline_eval(splineakima[phase_center], sec, acc[phase_center]);
  sec = (time - clock_epochs[clock_nr]).get_time();
  double clock_drift = clock_offsets[clock_nr] + sec * clock_rates[clock_nr];
  return result + clock_drift;
}

double Delay_table_akima::rate(const Time &time, int phase_center) {
  while (scans[scan_nr].end < time){
    if (!initialise_next_scan()) break;
  }

  SFXC_ASSERT(splineakima.size() > 0);
  double sec = (time - scans[scan_nr].begin).get_time();
  double result = gsl_spline_eval_deriv(splineakima[phase_center], sec, acc[phase_center]);
  return result + clock_rates[clock_nr];
}

double Delay_table_akima::phase(const Time &time, int phase_center){
  while (scans[scan_nr].end < time){
    if (!initialise_next_scan()) break;
  }
  SFXC_ASSERT(splineakima.size() > phase_center);
  double sec = (time - scans[scan_nr].begin).get_time();
  double result = gsl_spline_eval(splineakima_ph[phase_center], sec, acc_ph[phase_center]);
  return result;
}

double Delay_table_akima::amplitude(const Time &time, int phase_center){
  while (scans[scan_nr].end < time){
    if (!initialise_next_scan()) break;
  }
  SFXC_ASSERT(splineakima.size() > phase_center);
  double sec = (time - scans[scan_nr].begin).get_time();
  double result = gsl_spline_eval(splineakima_amp[phase_center], sec, acc_amp[phase_center]);
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

bool Delay_table_akima::goto_scan(const Time &time){
  if((time >= start_time_scan()) && (time < stop_time_scan()))
    return true;

  if(time < start_time_scan())
    scan_nr = 0;
  for(int i = 0 ; i < splineakima.size() ; i++){
    gsl_spline_free(splineakima[i]);
    gsl_interp_accel_free(acc[i]);
  }
  splineakima.resize(0);
  acc.resize(0);
  while ((scan_nr < scans.size()) && (time > scans[scan_nr].end)) 
    scan_nr++;
  if(scan_nr < scans.size()){
    initialise_next_scan();
    return true;
  }
  return false;
}

std::ostream &
operator<<(std::ostream &out, const Delay_table_akima &delay_table) {
  const std::vector<double> &times = delay_table.times;
  const std::vector<double> &delays = delay_table.delays;
  const std::vector<std::string> &sources = delay_table.sources;

  for (int i = 0; i < delay_table.scans.size(); i++) {
    const Delay_table_akima::Scan &scan = delay_table.scans[i];
    out << "scan " << i << ": start = " << scan.begin << ", stop = " << scan.end 
        << ", source = " << sources[scan.source] << std::endl;
    const Time one_sec(1000000);
    int n = (scan.end - scan.begin) / one_sec;
    for (int k = 0; k <= n; k++) {
      out << " t = " << times[scan.times + k] << " \t delay =" << delays[scan.delays + k] << std::endl;
    }
  }
  return out;
}
