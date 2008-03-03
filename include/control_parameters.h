#ifndef CONTROL_PARAMETERS_H_
#define CONTROL_PARAMETERS_H_

#include <json/json.h>
#include <vex/Vex++.h>
#include <list>
#include <algorithm>
#include "utils.h"



/** Information about the mark4 tracks needed by the input node. **/
class Input_node_parameters {
public:
  Input_node_parameters() 
    : track_bit_rate(0), number_channels(-1), integr_time(-1) {}

  class Channel_parameters {
  public:
    Channel_parameters() : sign_headstack(-1), magn_headstack(-1) {}


    bool operator==(const Channel_parameters &other) const;

    // Bits_per_sample is 1 if magn_tracks.size() == 0, otherwise 2
    int bits_per_sample() const;            ///< Number of bits to encode one sample
    int32_t              sign_headstack;   ///< The headstack for the sign bits
    std::vector<int32_t> sign_tracks;      ///< A list of the track numbers for sign
    int32_t              magn_headstack;   ///< The headstack for the magn bits
    std::vector<int32_t> magn_tracks;      ///< A list of the track numbers for magn
  };

  typedef std::vector<Channel_parameters>           Channel_list;
  typedef Channel_list::iterator                    Channel_iterator;
  typedef Channel_list::const_iterator              Channel_const_iterator;

  int bits_per_sample() const;
  int subsamples_per_sample() const;
  bool operator==(const Input_node_parameters &other) const;

  /// List of the tracks that are combined to frequency channels
  Channel_list                                 channels;
  // data
  int32_t                                     track_bit_rate; // in Ms/s
  /// number of frequency channels (#samples per output data chunk)
  int32_t number_channels;
  /// The integration time
  int32_t integr_time;
};

std::ostream &operator<<(std::ostream &out, const Input_node_parameters &param);


/** Information about the correlation neede by the correlator node. **/
class Correlation_parameters {
public:
  Correlation_parameters()
    : start_time(0), stop_time(0), integration_time(0),
      number_channels(0), integration_nr(-1), slice_nr(-1), slice_offset(-1), 
      sample_rate(0), bits_per_sample(0), channel_freq(0), bandwidth(0), 
      sideband('n'), channel_nr(0), polarisation('n') {
  }


  bool operator==(const Correlation_parameters& other) const;

  class Station_parameters {
  public:
    bool
    operator==(const Correlation_parameters::Station_parameters& other) const;

    int32_t station_number; // the number of the station 
                            // according to the vex file 
                            // sorted alphabathically
    int32_t station_stream; // input stream (from multiple_data_readers)
    int32_t start_time;     // Start and stop time for the station
    int32_t stop_time;
  };

  typedef std::vector<Station_parameters> Station_list;
  typedef Station_list::iterator          Station_iterator;

  // Data members
  int32_t start_time;       // Start of the slice in milliseconds
  int32_t stop_time;        // End of the slice in milliseconds
  int32_t integration_time; // In milliseconds
  int32_t number_channels;  // number of frequency channels
  int32_t integration_nr;   // number of the integration
  int32_t slice_nr;         // Number of the output slice
  int32_t slice_offset;     // Number of output slices in the output file
                            // between one integration slice and the next 
                            // in case of subsecond integrations
  int32_t sample_rate;      // #Samples per second
  int32_t bits_per_sample;  // For all stations equal
  int64_t channel_freq;     // Center frequency of the band in Hz
  int32_t bandwidth;        // Bandwidth of the channel in Hz
  char    sideband;         // U or L
  int32_t channel_nr;          // channel number ordered in the list
  char    polarisation;         // L or R

  bool    cross_polarize;   // do the cross polarisations
  int32_t reference_station;// use a reference station


  Station_list station_streams; // input streams used
};

std::ostream &operator<<(std::ostream &out, const Correlation_parameters &param);


/** Class containing all control variables needed for the experiment **/
class Control_parameters {
public:
  typedef Vex::Date                Date;


  Control_parameters();
  Control_parameters(const char *ctrl_file, const char *vex_file,
                     std::ostream& log_writer);

  bool initialise(const char *ctrl_filename,
                  const char *vex_filename,
                  std::ostream& log_writer);

  bool check(std::ostream &log_writer) const;

  /****************************************************/
  /* Get functions from the correlation control file: */
  /****************************************************/
  Date get_start_time() const;
  Date get_stop_time() const;
  std::vector<std::string> data_sources(const std::string &station) const;
  std::string get_output_file() const;

  std::string station(int i) const;
  size_t number_stations() const;

  int integration_time() const; // Integration time in miliseconds
  int number_channels() const;

  std::string sideband(int i) const;
  std::string reference_station() const;
  int reference_station_number() const;
  std::string experiment() const;

  std::string get_delay_directory() const;
  std::string get_delay_table_name(const std::string &station_name) const;
  void generate_delay_table(const std::string &station_name,
                            const std::string &filename) const;

  std::string channel(int i) const;
  size_t channels_size() const;

  int message_level() const;

  /****************************************************/
  /* Get functions from the vex file:                 */
  /****************************************************/
  int bits_per_sample() const;

  std::string scan(int i) const;
  int scan(const Date &date) const;
  size_t number_scans() const;


  std::string station_in_scan(const std::string& scan, int i) const;
  size_t number_stations_in_scan(const std::string& scan) const;
  int station_in_scan(const std::string& scan,
                      const std::string &station) const;

  // Takes cross polarisation into account
  int number_correlation_cores_per_timeslice(const std::string &mode) const;

  // Return the Frequency channels from the VEX file, filtered by the ctrl file
  size_t number_frequency_channels() const;
  std::string frequency_channel(size_t channel_nr) const;

  bool cross_polarize() const;
  int cross_channel(int channel_nr, 
                    const std::string &mode) const;
  int cross_channel(const std::string &channel_nr,
                    const std::string &mode) const;

  std::string get_mode(int32_t &time) const;

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
  (int integration_time,
   int data_rate,
   int number_channels) {
    return (integration_time * (data_rate / 1000) / number_channels);
  }

  /**
   * Computes the number of bytes transferred from the input node to the 
   * correlator node for one integration slice.
   **/
  static int nr_bytes_per_integration_slice_input_node_to_correlator_node
  (int integration_time,
   int data_rate,
   int bits_per_sample,
   int number_channels) {
    int nr_ffts = nr_ffts_per_integration_slice(integration_time,
                  data_rate,
                  number_channels);
    return nr_ffts*number_channels*bits_per_sample/8;
  }

  /**
   * Computes the number of bytes transferred from the input node to the 
   * correlator node for one integration slice.
   **/
  int integrations_per_timeslice(int duration_time_slice) {
    assert(duration_time_slice >= integration_time());
    assert(duration_time_slice % integration_time() == 0);
    return duration_time_slice/integration_time();
  }

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
                             const std::string &channel_name,
                             const std::vector<std::string> &station_name,
                             const std::map<std::string, int> &correlator_node_station_to_input) const;

  const Vex &get_vex() const;
private:
  std::string create_path(const std::string &path) const;
private:
  std::string ctrl_filename;
  std::string vex_filename;

  Json::Value ctrl;        // Correlator control file
  Vex         vex;         // Vex file
  bool        initialised; // The control parameters are initialised

};


#endif /*CONTROL_PARAMETERS_H_*/
