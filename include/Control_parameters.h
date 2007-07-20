#ifndef CONTROL_PARAMETERS_H_
#define CONTROL_PARAMETERS_H_

#include <Log_writer.h>
#include <string>
#include <map>
#include <list>

/** A class containing information about one station. **/
class Station_parameters {
public:
  Station_parameters();
  void add_data_source(const std::string &source);
  
private:
  std::list<std::string> data_sources;
};


/** Class containing all control variables needed for the experiment **/
class Control_parameters
{
  typedef std::map<std::string, Station_parameters> Station_map;
  typedef Station_map::iterator                     Station_map_it;
public:
  Control_parameters();
  
  bool initialise(char *ctrl_file, char *vex_file, Log_writer& log_writer);
private:
  bool parse_control_file(char *ctrl_filename, Log_writer& log_writer);
  bool parse_vex_file(char *vex_filename, Log_writer& log_writer);
  
  Station_parameters & get_station(const std::string &station);
  
  
  // Data
  std::string exper_name; // Name of the experiment
  int start_day;          // Day the experiment starts in MJD
  int start_time;         // Start time in seconds from midnight on start_day
  int stop_time;          // Stop time in seconds from midnight on start_day
  
  Station_map stations;           // Map from the stations to station data
  std::string reference_station;  // The reference station, if defined 
  
  bool cross_polarize;            // Compute cross polarisation pairs
  int number_channels;            // Number of frequency channels
  double integr_time;             // The integration time in seconds
  
  int message_level;              // The message level of the correlator

  std::string delay_directory;    // The directory containing the delay files
  std::string output_file;        // The file to store the output to

  
  bool initialised; // The control parameters are initialised
};

#endif /*CONTROL_PARAMETERS_H_*/
