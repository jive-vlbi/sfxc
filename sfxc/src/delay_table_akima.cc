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

#define READ_SCAN_HEADER	0
#define FIND_TSTART		1
#define START_NEW_SCAN		2
#define READ_PHASE_CENTER	3

//*****************************************************************************
//function definitions
//*****************************************************************************

Delay_table_akima::Delay_table_akima() {
  ref_count = new int(1);
  mutex = new pthread_mutex_t;
  pthread_mutex_init(mutex, NULL);
}

Delay_table_akima::~Delay_table_akima() {
  free_reference();
}

Delay_table_akima::Delay_table_akima(const Delay_table_akima &other) {
  copy(other);
}

void
Delay_table_akima::free_reference() {
  pthread_mutex_lock(mutex);
  *ref_count -= 1;
  if (*ref_count == 0) {
    for (int i = 0 ; i < splineakima.size(); i++) {
      gsl_spline_free(splineakima[i]);
      gsl_spline_free(splineakima_ph[i]);
      gsl_spline_free(splineakima_amp[i]);
      gsl_interp_accel_free(acc[i]);
      gsl_interp_accel_free(acc_ph[i]);
      gsl_interp_accel_free(acc_amp[i]);
    }
    pthread_mutex_unlock(mutex);
    pthread_mutex_destroy (mutex);
    delete mutex;
    delete ref_count;
  } else {
   pthread_mutex_unlock(mutex);
  }
  sources.resize(0);
  splineakima.resize(0);
  splineakima_ph.resize(0);
  splineakima_amp.resize(0);
  acc.resize(0);
  acc_ph.resize(0);
  acc_amp.resize(0);
}

void
Delay_table_akima::copy(const Delay_table_akima &other) {
  pthread_mutex_lock(other.mutex);
  mutex = other.mutex;
  ref_count = other.ref_count;
  *ref_count += 1;
  clock_epoch = other.clock_epoch;
  clock_offset = other.clock_offset;
  clock_rate = other.clock_rate;
  splineakima = other.splineakima;
  splineakima_ph = other.splineakima_ph;
  splineakima_amp = other.splineakima_amp;
  acc = other.acc;
  acc_ph = other.acc_ph;
  acc_amp = other.acc_amp;
  interval_begin = other.interval_begin;
  interval_end = other.interval_end;
  scan_begin = other.scan_begin;
  sources = other.sources;
  pthread_mutex_unlock(mutex);
}

void Delay_table_akima::operator=(const Delay_table_akima &other) {
  free_reference();
  copy(other);
}

//calculates the delay for the delayType at time
//get the next line from the delay table file
double Delay_table_akima::delay(const Time &time, int phase_center) {
  SFXC_ASSERT(time >= interval_begin);
  SFXC_ASSERT(time <= interval_end);
  SFXC_ASSERT(splineakima.size() > 0);

  double sec = time.diff(scan_begin);
  //std::cerr << RANK_OF_NODE << ", t = " << time << ", interval = " << interval_begin << " - " << interval_end 
  //          << ", scan_begin = " << scan_begin << ", sec = " << sec << "\n";
  double result = gsl_spline_eval(splineakima[phase_center], sec, acc[phase_center]);
  sec = time.diff(clock_epoch);
  double clock_drift = clock_offset + sec * clock_rate;
  return result + clock_drift;
}

double Delay_table_akima::rate(const Time &time, int phase_center) {
  SFXC_ASSERT(time >= interval_begin);
  SFXC_ASSERT(time <= interval_end);
  SFXC_ASSERT(splineakima.size() > 0);

  double sec = time.diff(scan_begin);
  double result = gsl_spline_eval_deriv(splineakima[phase_center], sec, acc[phase_center]);
  return result + clock_rate;
}

double Delay_table_akima::accel(const Time &time, int phase_center) {
  SFXC_ASSERT(time >= interval_begin);
  SFXC_ASSERT(time <= interval_end);
  SFXC_ASSERT(splineakima.size() > 0);

  double sec = time.diff(scan_begin);
  double result = gsl_spline_eval_deriv2(splineakima[phase_center], sec, acc[phase_center]);
  return result;
}

double Delay_table_akima::phase(const Time &time, int phase_center) {
  SFXC_ASSERT(time >= interval_begin);
  SFXC_ASSERT(time <= interval_end);
  SFXC_ASSERT(splineakima.size() > phase_center);

  double sec = time.diff(scan_begin);
  double result = gsl_spline_eval(splineakima_ph[phase_center], sec, acc_ph[phase_center]);
  return result;
}

double Delay_table_akima::amplitude(const Time &time, int phase_center) {
  SFXC_ASSERT(time >= interval_begin);
  SFXC_ASSERT(time <= interval_end);
  SFXC_ASSERT(splineakima.size() > phase_center);

  double sec = time.diff(scan_begin);
  double result = gsl_spline_eval(splineakima_amp[phase_center], sec, acc_amp[phase_center]);
  return result;
}

// Default constructor
Delay_table::Delay_table()
  : scan_nr(0), clock_nr(0), n_sources_in_current_scan(0) {
    // Add default clock offset
    set_clock_offset(0., 0., 0., 0.);
}

// Copy constructor
Delay_table::Delay_table(const Delay_table &other)
  : scan_nr(0), clock_nr(0), n_sources_in_current_scan(0) {
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
Delay_table::~Delay_table() {}

void Delay_table::operator=(const Delay_table &other) {
  scan_nr = 0;
  n_sources_in_current_scan = 0;
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

bool Delay_table::operator==(const Delay_table &other) const {
  if (scans.size() != other.scans.size())
    return false;
  for (int i = 0 ; i < delays.size(); i++) {
    if (delays[i] != other.delays[i])
      return false;
  }
  return true;
}

// Set clock offset and rate
void Delay_table::set_clock_offset(const double offset, const Time start, const double rate, const Time epoch) {
  clock_starts.push_back(start);
  clock_offsets.push_back(offset);
  clock_rates.push_back(rate);
  clock_epochs.push_back(epoch);
  // We don't do clock jumps in the middle of a scan
  if (scans.size() > 0 && start <= scans[scan_nr].begin)
    clock_nr = clock_starts.size() - 1;
}

//read the delay table, do some checks and
//calculate coefficients for parabolic interpolation
void Delay_table::open(const char *delayTableName) {
  const Time start, stop = Time::max_time();
  open(delayTableName, start, stop);
}

void Delay_table::open(const char *delayTableName, const Time tstart, const Time tstop) {
  std::ifstream in(delayTableName);
  if (!in.is_open())
    sfxc_abort((std::string("Could not open delay table ")+std::string(delayTableName)).c_str());

  int32_t header_size;
  // Read the header
  in.read(reinterpret_cast<char *>(&header_size), sizeof(int32_t));
  if (in.eof()) return;

  char header[header_size];
  in.read(reinterpret_cast<char *>(header), header_size * sizeof(char));
  if (in.eof()) return;

  double line[7], scan_start, scan_end;
  int32_t current_mjd;
  char current_source[81];

  int correlation_scan = 0;
  int state = READ_SCAN_HEADER;
  bool done_reading = false;
  while (!done_reading && in.good()) {
    switch (state) {
    case READ_SCAN_HEADER:{
      if (in.read(current_source, sizeof(char) * 81)) {
        in.read(reinterpret_cast<char *>(&current_mjd), sizeof(int32_t));
        in.read(reinterpret_cast<char *>(line), 7 * sizeof(double));
        Time start_time_scan(current_mjd, line[0]);
        // strip whitespace from end of source string
        for (int i = 79; i >= 0; i--) {
          if (current_source[i] != ' ') {
            current_source[i + 1] = 0;
            break;
          }
        }
        if (start_time_scan < tstart) {
          state = FIND_TSTART;
        } else if (start_time_scan >= tstop) {
          done_reading = true;
        } else {
          state = START_NEW_SCAN;
        }
      }
      break;
    }
    case FIND_TSTART:{
      while (in.read(reinterpret_cast<char *>(line), 7 * sizeof(double))) {
        Time time(current_mjd, line[0]);

        if (line[0] == 0 && line[4] == 0) {
          state = READ_SCAN_HEADER;
          break;
        } else if (time >= tstart) {
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
      for (source_index = 0; source_index < sources.size(); source_index++) {
        if (sources[source_index] == current_source)
          break;
      }
      if (source_index == sources.size())
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
      if (n_scans > 1 && scans[n_scans - 2].begin == start_time_scan) {
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
        if (line[0] == 0 && line[4] == 0) {
          if (times.size() == 1) {
            // Instead of the first point of the desired scan, we got the
            // last point of the previous scan.  Get rid of it.
            scans.resize(0);
            times.resize(0);
          } else {
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
      } while (in.read(reinterpret_cast<char *>(line), 7 * sizeof(double)));
      correlation_scan = scans.size() - 1;
      break;
    }}
  }
  // Initialise
  scan_nr = 0;
  n_sources_in_current_scan = 0;
  initialise_next_scan();
}

void
Delay_table::add_scans(const Delay_table &other)
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

bool Delay_table::initialise_next_scan() {
  int n_sources_in_previous_scan = n_sources_in_current_scan;
  if (scan_nr >= (scans.size() - n_sources_in_previous_scan))
    return false;

  scan_nr += n_sources_in_previous_scan;
  // Check scan length (need at least 5 points)
  if (scans[scan_nr].end.diff(scans[scan_nr].begin) < 4.)
    return false;

  // Determine the number of sources in current scan
  n_sources_in_current_scan = 1;
  while (scan_nr + n_sources_in_current_scan < scans.size() &&
	 scans[scan_nr + n_sources_in_current_scan - 1].begin == scans[scan_nr + n_sources_in_current_scan].begin)
    n_sources_in_current_scan++;
  clock_nr = clock_starts.size() - 1;
  while (clock_nr > 0 && clock_starts[clock_nr] > scans[scan_nr].begin)
    clock_nr--;
  
  return true;
}

Delay_table_akima
Delay_table::create_akima_spline(const Time start_, const Time duration) {
  // Round begin and end time to integer second
  int mjd_start = (int)floor(start_.get_mjd());
  double seconds_start = floor(start_.get_time_usec() / 1000000);
  Time stop = start_ + duration;
  int mjd_stop = (int)floor(stop.get_mjd());
  double seconds_stop = ceil(stop.get_time_usec() / 1000000);
  Time start = Time(mjd_start, seconds_start);
  int n_seconds = (mjd_stop - mjd_start) * 86400 + seconds_stop - seconds_start;
  // Move to the correct scan
  while (stop > scans[scan_nr].end) {
    if (!initialise_next_scan()) {
      std::string msg = "Error: time " + start.date_string() + " is not in delay table";
      sfxc_abort(msg.c_str());
    }
  }

  // Determine interpolation interval
  Time onesec(1000000.);
  Time interval_begin = start - onesec * 2;
  Time interval_end = start + onesec * (2 + n_seconds);
  if (interval_begin < scans[scan_nr].begin) {
    interval_begin = scans[scan_nr].begin;
    if (interval_end.diff(interval_begin) < 4)
      interval_end = interval_begin + onesec * 4;
  }
  if (interval_end > scans[scan_nr].end) {
    interval_end = scans[scan_nr].end;
    if (interval_end.diff(interval_begin) < 4)
      interval_begin = interval_end - onesec * 4;
  }

  // Create the splines
  Delay_table_akima result;
  result.splineakima.resize(n_sources_in_current_scan);
  result.splineakima_ph.resize(n_sources_in_current_scan);
  result.splineakima_amp.resize(n_sources_in_current_scan);
  result.acc.resize(n_sources_in_current_scan);
  result.acc_ph.resize(n_sources_in_current_scan);
  result.acc_amp.resize(n_sources_in_current_scan);
  result.sources.resize(n_sources_in_current_scan);
  SFXC_ASSERT(n_sources_in_current_scan > 0);
  for (int i = 0; i < n_sources_in_current_scan; i++) {
    Scan &scan = scans[scan_nr + i];
    result.sources[i] = sources[scan.source];

    // at least 4 sample points for a spline
    int n_pts = (interval_end - interval_begin) / onesec + 1;
    int idx = (int)interval_begin.diff(scan.begin);
    SFXC_ASSERT(n_pts > 4);

    // Initialise the Akima spline
    result.acc[i] = gsl_interp_accel_alloc();
    result.acc_ph[i] = gsl_interp_accel_alloc();
    result.acc_amp[i] = gsl_interp_accel_alloc();
    result.splineakima[i] = gsl_spline_alloc(gsl_interp_akima, n_pts);
    result.splineakima_ph[i] = gsl_spline_alloc(gsl_interp_akima, n_pts);
    result.splineakima_amp[i] = gsl_spline_alloc(gsl_interp_akima, n_pts);

    gsl_spline_init(result.splineakima[i], &times[scan.times+idx], &delays[scan.delays+idx], n_pts);
    gsl_spline_init(result.splineakima_ph[i], &times[scan.times+idx], &phases[scan.phases+idx], n_pts);
    gsl_spline_init(result.splineakima_amp[i], &times[scan.times+idx], &amplitudes[scan.amplitudes+idx], n_pts);
  }

  result.scan_begin = scans[scan_nr].begin;
  result.interval_begin = interval_begin;
  result.interval_end = interval_end;
  result.clock_epoch = clock_epochs[clock_nr];
  result.clock_offset = clock_offsets[clock_nr];
  result.clock_rate = clock_rates[clock_nr];
  return result;
}

Time Delay_table::start_time_scan() {
  SFXC_ASSERT(scan_nr < scans.size());
  return scans[scan_nr].begin;
}

Time Delay_table::stop_time_scan() {
  SFXC_ASSERT(scan_nr < scans.size());
  return scans[scan_nr].end;
}

std::ostream &
operator<<(std::ostream &out, const Delay_table &delay_table) {
  const std::vector<double> &times = delay_table.times;
  const std::vector<double> &delays = delay_table.delays;
  const std::vector<std::string> &sources = delay_table.sources;

  for (int i = 0; i < delay_table.scans.size(); i++) {
    const Delay_table::Scan &scan = delay_table.scans[i];
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
