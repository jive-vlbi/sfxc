/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 *
 * $Id: delayTable.h 278 2007-07-04 07:27:05Z kruithof $
 *
 * Class definitions for delay table
 */
#ifndef GENERATE_DELAY_MODEL_H
#define GENERATE_DELAY_MODEL_H

struct Station_data {
  // Station information.
  char   site_name[9];
  double site_position[3]; // m
  int    axis_type; // az : el
  double axis_offset; // m
  double clock_early; //sec
  double clock_rate; //sec/sec
  double clock_epoch; //sec

  // EOP information.
  double tai_utc; // s
  double eop_ref_epoch; // julian day
  int    num_eop_points;
  double ut1_utc[10];  // sec
  double x_wobble[10]; // asec
  double y_wobble[10]; // asec

};

struct Source_data{
  // Source information.
  char   source_name[81];
  double ra; // rad
  double dec; // rad
};

struct Scan_data {
  // Observation time.
  int    year, month, day, hour, min;
  double sec, sec_of_day;
  double scan_start, scan_stop;
  int nr_of_intervals;
  // All sources in the current scan
  int n_sources;
  struct Source_data **sources;
};

extern const double delta_time; //unit: seconds

// Station related data
extern struct Station_data station_data;
// Number of scans
extern int n_scans;
// Number of sources
extern int n_sources;
// All sources in experiment
extern struct Source_data *source_data;
// Data per scan
extern struct Scan_data *scan_data;
#endif // GENERATE_DELAY_MODEL_H
