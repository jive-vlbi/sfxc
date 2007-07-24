#ifndef CONTROL_PARAMETERS_H_
#define CONTROL_PARAMETERS_H_

#include <Log_writer.h>
#include <string>
#include <map>
#include <list>
#include <vexplus.h>

/** A class containing information about one station. **/
class Station_parameters {
public:
  Station_parameters();
  void add_data_source(const std::string &source);

  Log_writer &operator<<(Log_writer &log_writer);
  
private:
  std::list<std::string> data_sources;
};

/** A class containing information about one scan. **/
class Scan_parameters {
public:
  struct Scan_station_parameters {
    std::string station_name;
    int start_second, stop_second;
    std::string frequency;

    Log_writer &operator<<(Log_writer &log_writer);
  };

  Scan_parameters() {}
  
  void set_name(std::string new_name) { name = new_name; }
  
  Log_writer &operator<<(Log_writer &log_writer);

  std::string name;
  int         start;
  
  std::list<Scan_station_parameters> stations;
};

/** A class containing information about a frequency setup. **/
class Frequency_parameters {
public:
  struct Frequency_channel_parameters {
    std::string channel_name; ///< Name of the channel
    double      frequency;    ///< Frequency
    char        sideband;     ///< Upper or lower sideband (U or L)
    char        polarisation; ///< The polarisation (L or R)
    double      band_width;   ///< Width of the frequency channel
    int         sign_headstack;    ///< The headstack for the sign bits
    std::list<int> sign_tracks;    ///< A list of the track numbers for sign
    int         magn_headstack;    ///< The headstack for the magn bits
    std::list<int> magn_tracks;    ///< A list of the track numbers for magn
    
    Log_writer &operator<<(Log_writer &log_writer);
  };
  
  Frequency_parameters() {}
  
  Log_writer &operator<<(Log_writer &log_writer);
  
  std::string                             frequency_name; 
  int                                     sample_rate;
  std::list<Frequency_channel_parameters> channels;
  
};

/** Class containing all control variables needed for the experiment **/
class Control_parameters
{
  typedef std::map<std::string, Station_parameters>   Station_map;
  typedef Station_map::iterator                       Station_map_it;
  typedef std::map<int, Scan_parameters>              Scan_map;
  typedef Scan_map::iterator                          Scan_map_it;
  typedef std::map<std::string, Frequency_parameters> Frequency_map;
  typedef Frequency_map::iterator                     Frequency_map_it;
public:
  Control_parameters();
  
  bool initialise(char *ctrl_file, char *vex_file, Log_writer& log_writer);

  // Access operators
  bool is_station(const std::string &station);
  Station_parameters & get_station(const std::string &station);

  
  Log_writer &operator<<(Log_writer &log_writer);
private:
  bool parse_control_file(char *ctrl_filename, Log_writer& log_writer);
  bool parse_vex_file(char *vex_filename, Log_writer& log_writer);
  
  bool set_scan_information(VexPlus &vex_file, Log_writer &log_writer); 
  
  
  int get_day(const char *time);
  int get_time(const char *time);
  
  // General
  std::string exper_name; // Name of the experiment
  int start_day;       // Day the experiment starts in MJD
  int start_time;      // Start time in miliseconds from midnight on start_day
  int stop_time;       // Stop time in miliseconds from midnight on start_day
  int message_level;   // The message level of the correlator

  // Station
  Station_map stations;           // Map from the stations to station data
  std::string reference_station;  // The reference station, if defined 
  
  // Scans
  Scan_map scans;                 // A map for the scans
  Frequency_map frequencies;      // A map of the frequencies and tracks
  
  // Correlation
  bool cross_polarize;            // Compute cross polarisation pairs
  int number_channels;            // Number of frequency channels
  double integr_time;             // The integration time in seconds

  // Data  
  std::string delay_directory;    // The directory containing the delay files
  std::string output_file;        // The file to store the output to

  
  bool initialised; // The control parameters are initialised
};

Log_writer &operator<<(Log_writer &log_writer, Control_parameters &control);
Log_writer &operator<<(Log_writer &log_writer, Station_parameters &station);
Log_writer &operator<<(Log_writer &log_writer, Scan_parameters &scan);
Log_writer &operator<<(Log_writer &log_writer, 
                       Scan_parameters::Scan_station_parameters &scan);
Log_writer &operator<<(Log_writer &log_writer, Frequency_parameters &scan);
Log_writer &
operator<<(Log_writer &log_writer, 
           Frequency_parameters::Frequency_channel_parameters &scan);

#endif /*CONTROL_PARAMETERS_H_*/
