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
    start_year : int16_t
    start_day  (day of year): int16_t
    start_time (in seconds since midnight on the startday): int32_t
    number_channels (number of visibilities): int32_t
    integration_time (in micro seconds): int32_t (signed)
  )
  (
    ( # one timeslice
      integration_slice
       (time = start_time + integration_slice*2^integration_time): int32_t
      number_baselines: int32_t
      number_uvw_coordinates: int32_t
      number_statistics: int32_t
      (
        station_nr : int32_t
        u,v,w : double
      ){number_uvw_coordinates times}
      (
        # The bit statistics
        station_nr: uint8_t
        frequency_nr: uint8_t
        sideband:uint8_t (LSB: 0, USB: 1)
        polarisation:uint8_t (RCP: 0, LCP: 1)
        levels[4] : int32_t[4]
        n_invalid : int32_t
      ){number_statistics times}
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

#define OUTPUT_FORMAT_VERSION  1

struct Output_header_global {
  Output_header_global()
      : header_size(0),
      start_year(0), start_day(0), start_time(0),
  number_channels(0), integration_time(0), polarisation_type(0) {
    experiment[0] = '\0';
  }

  int32_t header_size;      // Size of the global header in bytes
  char experiment[32];      // Name of the experiment
  int16_t start_year;       // Start year of the experiment
  int16_t start_day;        // Start day of the experiment (day of year)
  int32_t start_time;       // Start time of the correlation in seconds since
  // midnight
  int32_t number_channels;  // Number of frequency channels
  int32_t integration_time; // Integration time in micro seconds

  static const int LEFT_POLARISATION=0;
  static const int RIGHT_POLARISATION=1;
  static const int LEFT_RIGHT_POLARISATION=2;
  static const int LEFT_RIGHT_POLARISATION_WITH_CROSSES=3;

  int32_t output_format_version;  // Version number of the output format
  int32_t correlator_version;     // Svn revision of the correlator producing the output file

  int8_t polarisation_type; // L | R | L+R | L+R with crosses
  // 3 bytes left:
  int8_t empty[3];
};

struct Output_header_timeslice {
  Output_header_timeslice()
      : integration_slice(0), number_baselines(0), number_uvw_coordinates(0) {}
  int32_t integration_slice; // Integration slice number
  int32_t number_baselines;  // The number of baselines that follow
  int32_t number_uvw_coordinates; // The number of uvw coordinates that follow
  int32_t number_statistics;  // The number of bitstatistics that follow;
// int32_t polyco_nr; // Index of the polyco file used
};

struct Output_uvw_coordinates {
Output_uvw_coordinates() : station_nr(0), reserved(0), u(0), v(0), w(0) {}
  int32_t station_nr; // The station number in the vex-file
  int32_t reserved;   // Added for 64bit alignment
  double u, v, w;     // The u, v and w coordinates
};

struct Output_header_baseline {
  Output_header_baseline()
      : weight(-1), station_nr1(0), station_nr2(0),
      polarisation1(0), polarisation2(0),
  sideband(0), frequency_nr(0) {}
  int32_t weight;       ///< The number of good samples
  uint8_t station_nr1;  ///< Station number in the vex-file
  uint8_t station_nr2;  ///< Station number in the vex-file
unsigned char polarisation1:
  1; // Polarisation for the first station
  // (RCP: 0, LCP: 1)
unsigned char polarisation2:
  1; // Polarisation for the second station
unsigned char sideband:
  1;      // Upper or lower sideband
  // (LSB: 0, USB: 1)
unsigned char frequency_nr:
  5;  // The number of the channel in the vex-file,
  // sorted increasingly
  // 1 byte left:
  char empty;
};

struct Output_header_bitstatistics{
  uint8_t station_nr;   // Station number in the vex-file
  uint8_t frequency_nr; // The number of the channel in the vex-file
  uint8_t sideband;     // (LSB: 0, USB: 1)
  uint8_t polarisation; // (RCP: 0, LCP: 1)
  // order : -0 -1 +0 +1 ; for 1 bit data levels[0]=level[3]=0 ; here - means sign bit = 0
  int32_t levels[4];
  int32_t n_invalid;    // The number of invalid samples
};

struct Output_header_polyco {
  /// All parameters from the polyco file
  int32_t data_span;        // Data span [minutes]
  int32_t n_coef;           // The number of coefficients in the polynomial
  int32_t npblk;            // Nr of blocks(rows) in polyco
  char nsite[8];            // Observertary code
  double ref_freq;          // Reference frequency for phase
  double pred_phs;          // Predicted pulse phase at start of experiment
  double ref_mjd;           // Reference MJD
  double ref_phs;           // Reference phase
  double ref_F0;            // Zero'th order pulsar frequency[Hz]
  double logfiterr;         // Log_10 of polynomial fit error [periods]
  double coef[15];          // The polynomial coefficients
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

bool
operator<(const Output_header_timeslice &timeslice_header1,
          const Output_header_timeslice &timeslice_header2);
bool
operator<(const Output_header_baseline &baseline_header1,
          const Output_header_baseline &baseline_header2);

bool
operator==(const Output_header_timeslice &timeslice_header1,
           const Output_header_timeslice &timeslice_header2);
bool
operator==(const Output_header_baseline &baseline_header1,
           const Output_header_baseline &baseline_header2);

#endif /*OUTPUT_HEADER_H_*/
