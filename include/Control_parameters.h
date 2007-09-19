#ifndef CONTROL_PARAMETERS_H_
#define CONTROL_PARAMETERS_H_

#include <json/json.h>
#include <Vex++.h>


/** Information about the mark4 tracks needed by the input node. **/
class Track_parameters {
public:
  Track_parameters() {}
  class Channel_parameters {
  public:
    Channel_parameters()
      : sign_headstack(-1), 
        magn_headstack(-1) {
    }

    // Bits_per_sample is 1 if magn_tracks.size() == 0, otherwise 2
    int bits_per_sample() const;      ///< Number of bits to encode one sample
    int32_t         sign_headstack;   ///< The headstack for the sign bits
    std::vector<int32_t> sign_tracks; ///< A list of the track numbers for sign
    int32_t         magn_headstack;   ///< The headstack for the magn bits
    std::vector<int32_t> magn_tracks; ///< A list of the track numbers for magn
  };

  typedef std::map<std::string,Channel_parameters> Channel_map;
  typedef Channel_map::iterator                    Channel_iterator;
  typedef Channel_map::const_iterator              Channel_const_iterator;

  int number_of_channels() const;
  int                                         sample_rate; // in Ms/s
  std::map<std::string,Channel_parameters>    channels;
};


/** Information about the mark4 tracks needed by the input node. **/
class Correlation_parameters {
public:
  int32_t start_time;       // Start of the slice in milliseconds
  int32_t stop_time;        // End of the slice in milliseconds
  int32_t integration_time; // In milliseconds
  int32_t number_channels;  // number of frequency channels

  int32_t data_rate;        // Bytes per second
  int32_t bits_per_sample;  // For all stations equal

  class Station_parameters {
    int32_t station_streams; // input stream
    int32_t start_time;      // Start and stop time for the station
    int32_t stop_time;
  };
  std::vector<Station_parameters> station_streams; // input streams used
};


/** Class containing all control variables needed for the experiment **/
class Control_parameters
{
public:
  typedef Vex::Date                Date;


  Control_parameters();
  Control_parameters(const char *ctrl_file, const char *vex_file, 
                     std::ostream& log_writer);
  
  bool initialise(const char *ctrl_filename,
                  const char *vex_filename, 
                  std::ostream& log_writer);

  // Get functions from the correlation control file:
  Date get_start_time();
  Date get_stop_time();
  std::vector<std::string> data_sources(const std::string &station) const;

  std::string station(int i) const;
  size_t number_stations() const;

  // Get functions from the vex file:

  // Return the track parameters needed by the input node
  Track_parameters 
  get_track_parameters(const std::string &track_name) const;
  
  // Return the correlation parameters needed by a correlator node
  Correlation_parameters 
  get_correlation_parameters(const std::string &scan_name) const;

  std::string get_delay_table_name(const std::string &station_name) const;

  const Vex &get_vex() const;
private:
  Json::Value ctrl;        // Correlator control file
  Vex         vex;         // Vex file
  bool        initialised; // The control parameters are initialised
};


#endif /*CONTROL_PARAMETERS_H_*/
