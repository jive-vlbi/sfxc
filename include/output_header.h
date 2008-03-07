#ifndef OUTPUT_HEADER_H_
#define OUTPUT_HEADER_H_

#include <stdint.h>
#include <iostream>

/*
  20-11-2007 added: 
    - Experiment name
    - Day
    - Year
    - uvw coordinates
*/

/*
  Schematic outline of the correlator output format.
  Author: Nico Kruithof
  
  -----------
  New Version
  
  ( # global header
    header_size (in bytes): int32_t
    char experiment[32];
    start_year : int32_t
    start_day  (day of year): int32_t
    start_time (in seconds since midnight on the startday): int32_t
    number_channels (number of visibilities): int32_t
    integration_time (in 2^integration_time seconds): int8_t (signed)
  )
  (
    ( # one timeslice
      integration_slice
       (time = start_time + integration_slice*2^integration_time): int32_t
      number_baselines: int32_t
      number_uvw_coordinates: int32_t
      (
        station_nr : int32_t
        u,v,w : double
      ){number_uvw_coordinates times}
    )
    ( # one baseline
      weight: int32_t
      station_nr1 : uint8_t
      station_nr2 : uint8_t
      polarisation1 (polarisation for station 1) : unsigned char:1 (RCP: 0, LCP: 1)
      polarisation2 (polarisation for station 2) : unsigned char:1 (RCP: 0, LCP: 1)

      sideband     : unsigned char:1 (LSB: 0, USB: 1)
      frequency_nr : unsigned char:5 (sorted increasingly)
    
      (real: float,
       imag: float){number_channels times}
    ){number_correlations times}
  )+
*/
  
struct Output_header_global {
  int32_t header_size;      // Size of the global header in bytes
  char experiment[32];      // Name of the experiment
  int32_t start_year;       // Start year of the experiment
  int32_t start_day;        // Start day of the experiment (day of year)
  int32_t start_time;       // Start time of the correlation in seconds since
                            // midnight
  int32_t number_channels;  // Number of frequency channels
  int8_t  integration_time; // Integration time: 2^integration_time seconds
  
  static const int LEFT_POLARISATION=0;
  static const int RIGHT_POLARISATION=1;
  static const int LEFT_RIGHT_POLARISATION=2;
  static const int LEFT_RIGHT_POLARISATION_WITH_CROSSES=3;
  
  int8_t polarisation_type; // L | R | L+R | L+R with crosses
  // 3 bytes left:
  int8_t empty[2];
};

struct Output_header_timeslice {
  int32_t integration_slice; // Integration slice number
  int32_t number_baselines;  // The number of baselines that follow
  int32_t number_uvw_coordinates; // The number of uvw coordinates that follow
};

struct Output_uvw_coordinates {
  int32_t station_nr; // The station number in the vex-file
  double u, v, w;     // The u, v and w coordinates
};

struct Output_header_baseline {
  int32_t weight;       // The number of good samples
  uint8_t station_nr1;  // Station number in the vex-file
  uint8_t station_nr2;  // Station number in the vex-file
  unsigned char polarisation1:1; // Polarisation for the first station
                                 // (RCP: 0, LCP: 1)
  unsigned char polarisation2:1; // Polarisation for the second station
  unsigned char sideband:1;      // Upper or lower sideband
                                 // (LSB: 0, USB: 1)
  unsigned char frequency_nr:5;  // The number of the channel in the vex-file,
                                 // sorted increasingly
  // 1 byte left:
  char empty;
};


std::ostream &
operator<<(std::ostream &out,
           const Output_header_global &global_header);
std::ostream &
operator<<(std::ostream &out,
           const Output_header_timeslice &timeslice_header);
std::ostream &
operator<<(std::ostream &out,
           const Output_uvw_coordinates &uvw_header);
std::ostream &
operator<<(std::ostream &out,
           const Output_header_baseline &baseline_header);

#endif /*OUTPUT_HEADER_H_*/
