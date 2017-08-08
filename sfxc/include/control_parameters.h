#ifndef CONTROL_PARAMETERS_H_
#define CONTROL_PARAMETERS_H_

#include <boost/shared_ptr.hpp>
#include <json/json.h>
#include <vex/Vex++.h>
#include <list>
#include <algorithm>
#include <map>
#include <math.h>
#include "utils.h"
#include "correlator_time.h"


/** Information about the mark5 tracks needed by the input node. **/
class Input_node_parameters {
public:
  Input_node_parameters()
      : track_bit_rate(0), fft_size(-1), data_modulation(0) {}

  class Channel_parameters {
  public:
    Channel_parameters() : bits_per_sample(-1) {}


    bool operator==(const Channel_parameters &other) const;

    int bits_per_sample;                   ///< Number of bits to encode one sample
    std::vector<int32_t> tracks;           ///< A list of bit positions for all tracks

    char sideband;
    char polarisation;
    int32_t frequency_number;
  };

  typedef std::vector<Channel_parameters>           Channel_list;
  typedef Channel_list::iterator                    Channel_iterator;
  typedef Channel_list::const_iterator              Channel_const_iterator;

  int bits_per_sample() const;
  int subsamples_per_sample() const;
  int sample_rate() const;
  bool operator==(const Input_node_parameters &other) const;

  /// List of the tracks that are combined to frequency channels
  Channel_list channels;
  /// Number of tracks / bitstreams in data file
  int32_t n_tracks;
  // data rate of one subband
  int32_t track_bit_rate; // in Ms/s
  // Frame size
  int32_t frame_size;
  // Number of samples per FFT.
  int32_t fft_size;
  /// The integration time
  Time integr_time;
  /// Time offset for the data reader, used e.g. to compensate for formatter errors
  Time offset;
  /// Indicates if data modulation is used (p.6 of Mark4 memo 230A, Whitney 2005)
  int32_t data_modulation;
  // Phasecal integration time
  Time phasecal_integr_time;
  // Abort the correlation if the input stream contains no valid data
  bool exit_on_empty_datastream;
};

std::ostream &operator<<(std::ostream &out, const Input_node_parameters &param);


class Pulsar_parameters {
public:
  Pulsar_parameters(std::ostream& log_writer_);
  /// All parameters from the polyco file
  struct Polyco_params{
    char name[11];            // Pulsar name
    char date[10];            // Date 
    float utc;                // UTC 
    double tmid;              // Reference time [mjd]
    double DM;                // Dispersion measure
    float doppler;            // Doppler shift due to earth motion (10^-4)
    float residual;           // Log10 of fit rms residual in periods
    double ref_phase;         // Reference phase (RPHASE)
    double ref_freq;          // Reference rotation frequency (F0)
    char site[6];             // Observatory name
    int32_t data_span;        // Data span [minutes]
    int32_t n_coef;           // The number of coefficients in the polynomial
    double obs_freq;          // Observing frequency
    float bin_phase[2];        // Binary phase
    std::vector<double> coef; // The polynomial coefficients
  };

  struct Pulsar{
    char name[11];
    int32_t nbins;
    struct Interval{double start; double stop;} interval;
    std::vector<Polyco_params> polyco_params;
  };

  bool parse_polyco(std::vector<Polyco_params> &param, std::string filename);

  // maps pulsar name to vector of pulsar pa
  std::map<std::string, Pulsar> pulsars;
private:
  std::ostream& log_writer;
};

class Mask_parameters {
 public:
   Mask_parameters() : normalize(false) {}

  bool normalize;
  std::vector<double> mask;
  std::vector<double> window;
};

/** Information about the correlation neede by the correlator node. **/
class Correlation_parameters {
public:
  Correlation_parameters()
      : number_channels(0), fft_size_delaycor(0), fft_size_correlation(0), integration_nr(-1), slice_nr(-1), 
        slice_offset(-1), sample_rate(0), channel_freq(0), bandwidth(0),
        sideband('n'), frequency_nr(-1), polarisation('n'), pulsar_binning(false), window(SFXC_WINDOW_RECT) {}


  bool operator==(const Correlation_parameters& other) const;

  class Station_parameters {
  public:
    bool
    operator==(const Correlation_parameters::Station_parameters& other) const;

    int32_t station_number; // the number of the station
    // according to the vex file
    // sorted alphabathically
    int32_t station_stream; // input stream (from multiple_data_readers)
    Time start_time;         // Start and stop time for the station
    Time stop_time;
    int32_t sample_rate;
    int64_t channel_freq;
    int32_t bandwidth;
    char sideband;
    char polarisation;
    int32_t bits_per_sample;
    double LO_offset; // LO offset in Hz
    int tsys_freq;
  };

  typedef std::vector<Station_parameters> Station_list;
  typedef Station_list::iterator          Station_iterator;

  // Data members
  Time experiment_start;    // Start time of the experiment
  Time start_time;          // Start of the slice in microseconds
  Time stop_time;           // End of the slice in microseconds
  Time integration_time;
  Time sub_integration_time;// The length of one sub integration
  int32_t number_channels;  // number of frequency channels
  int32_t fft_size_delaycor;    // Number of samples per FFT in the delay correction
  int32_t fft_size_correlation; // Number of samples per FFT in the (cross-)correlation
  int32_t integration_nr;   // number of the integration
  int32_t slice_nr;         // Number of the output slice
  int32_t slice_offset;     // Number of output slices in the output file
  // between one integration slice and the next
  // in case of subsecond integrations
  int32_t sample_rate;      // #Samples per second
  int64_t channel_freq;     // Center frequency of the band in Hz
  int32_t bandwidth;        // Bandwidth of the channel in Hz
  char    sideband;         // U or L
  int32_t frequency_nr;     // Canonical frequency number
  char    polarisation;     // L or R

  bool    cross_polarize;   // do the cross polarisations
  int32_t reference_station;// use a reference station

  Station_list station_streams; // input streams used
  int window;                   // Windowing function to be used
  char source[11];              // name of the source under observation
  int32_t n_phase_centers;   // The number of phase centers in the current scan
  int32_t pulsar_binning;
  Pulsar_parameters *pulsar_parameters;
  Mask_parameters *mask_parameters;
};


std::ostream &operator<<(std::ostream &out, const Correlation_parameters &param);

/** Class containing all control variables needed for the experiment **/
class Control_parameters {
public:

  Control_parameters();
  Control_parameters(const char *ctrl_file, const char *vex_file,
                     std::ostream& log_writer);

  bool initialise(const char *ctrl_filename,
                  const char *vex_filename,
                  std::ostream& log_writer);

  bool check(std::ostream &log_writer) const;
  bool get_pulsar_parameters(Pulsar_parameters &pars) const;
  bool get_mask_parameters(Mask_parameters &pars) const;

  /****************************************************/
  /* Get functions from the correlation control file: */
  /****************************************************/
  Time get_start_time() const;
  Time get_stop_time() const;
  std::vector<std::string> data_sources(const std::string &station) const;
  std::string get_output_file() const;
  std::string get_phasecal_file() const;
  std::string get_tsys_file() const;

  std::string station(int i) const;
  size_t number_stations() const;
  int station_number(const std::string &station_name) const;

  Time integration_time() const; // Integration time in microseconds
  Time sub_integration_time() const;
  Time phasecal_integration_time() const;
  int number_channels() const;
  int fft_size_delaycor() const;
  int fft_size_correlation() const;
  int window_function() const;
  int job_nr() const;
  int subjob_nr() const;

  std::string sideband(int i) const;
  std::string reference_station() const;
  int reference_station_number() const;
  std::string setup_station() const;

  bool phased_array() const;
  bool pulsar_binning() const;
  bool multi_phase_center() const;
  double LO_offset(const std::string &station) const;
  int tsys_freq(const std::string &station) const;
  bool exit_on_empty_datastream() const;
  
  Time reader_offset(const std::string &s) const{
    return reader_offsets.find(s)->second;
  };
  
  void set_reader_offset(const std::string &s, const Time t);

  std::string get_delay_table_name(const std::string &station_name) const;
  void generate_delay_table(const std::string &station_name,
                            const std::string &filename) const;
  std::string channel(int i) const;

  int message_level() const;

  /****************************************************/
  /* Get functions from the vex file:                 */
  /****************************************************/
  int bits_per_sample(const std::string& mode, const std::string& station) const;
  int sample_rate(const std::string& mode, const std::string& station) const;
  int bandwidth(const std::string& mode, const std::string& station, const std::string& channel) const;
  int64_t channel_freq(const std::string& mode, const std::string& station, const std::string& channel) const;

  std::string scan(int i) const;
  int scan(const Time &time) const;
  std::string scan_source(const std::string &scan) const;

  size_t number_scans() const;

  bool station_in_scan(const std::string& scan, const std::string &station) const;
  size_t number_stations_in_scan(const std::string& scan) const;

  Time stop_time(const std::string& scan, const std::string &station) const;

  // Takes cross polarisation into account
  int number_correlation_cores_per_timeslice(const std::string &mode) const;

  // Return the Frequency channels from the VEX file, filtered by the ctrl file
  size_t number_frequency_channels() const;
  std::string frequency_channel(size_t channel_nr, const std::string& mode_name, const std::string &station_name) const;
  int frequency_number(size_t channel_nr, const std::string& mode_name) const;

  bool cross_polarize() const;
  int cross_channel(int channel_nr,
                    const std::string &mode) const;
  int cross_channel(const std::string &channel_nr,
                    const std::string &mode) const;

  char polarisation(const std::string &channel_name,
                    const std::string &station_name,
                    const std::string &mode) const;

  std::string frequency(const std::string &channel_name,
                        const std::string &station_name,
                        const std::string &mode) const;

  char sideband(const std::string &channel_name,
                const std::string &station_name,
                const std::string &mode) const;

  /**
   * Returns the number of bytes transferred for one integration slice
   * from the input node to the correlator node.
   **/
  static int nr_ffts_per_integration_slice
  (const Time &integration_time,
   int data_rate,
   int fft_size) {
    Time time_one_fft(fft_size / (data_rate / 1000000.));
    return floor(integration_time / time_one_fft);
  }

  int polarisation_type_for_global_output_header(const std::string &mode) const;

  /****************************************************/
  /* Extract structs for the correlation:             */
  /****************************************************/

  // Return the track parameters needed by the input node
  Input_node_parameters
  get_input_node_parameters(const std::string &mode_name,
                            const std::string &station_name) const;

  // Return the correlation parameters needed by a correlator node
  Correlation_parameters
  get_correlation_parameters(const std::string &scan_name,
                             size_t channel_nr,
                             const std::map<std::string, int> &correlator_node_station_to_input) const;
  std::string rack_type(const std::string &station) const;
  std::string transport_type(const std::string &station) const;
  std::string data_format(const std::string &station) const;

  const Vex &get_vex() const;
  std::string get_exper_name() const;

  /// The start time of the first scan in the vex file
  Time start_time;

private:
  std::string create_path(const std::string &path) const;
private:

  // Gets the track parameters for mark5a data
  // Output is in input_parameters
  void get_mark5a_tracks(const std::string &mode,
                         const std::string &station,
                         Input_node_parameters &input_parameters) const;
  // Get the bit positions for all tracks in the vex file
  std::vector<int> get_track_bit_position(const std::string &mode, const std::string &station) const;
  int n_mark5a_tracks(const std::string &mode, const std::string &station) const;

  // Gets the track parameters for mark5b data
  // Output is in input_parameters
  void get_mark5b_tracks(const std::string &mode,
                         const std::string &station,
                         Input_node_parameters &input_parameters) const;
  int n_mark5b_bitstreams(const std::string &mode, const std::string &station) const;
  void get_mark5b_standard_mapping(const std::string &mode,
                                   const std::string &station,
                                   Input_node_parameters &input_parameters) const;

  // Gets the track parameters for VDIF data
  // Output is in input_parameters
  void get_vdif_tracks(const std::string &mode,
		       const std::string &station,
		       Input_node_parameters &input_parameters) const;

  std::string ctrl_filename;
  std::string vex_filename;
  std::map<std::string, Time> reader_offsets; // Contains the formatter clock offsets for all input nodes

  Json::Value ctrl;        // Correlator control file
  Vex         vex;         // Vex file
  bool        initialised; // The control parameters are initialised

  mutable std::map<std::string, int> station_map;
};

#endif /*CONTROL_PARAMETERS_H_*/
