#include "Vexpp_node.h"

#ifndef VEXPP_H
#define VEXPP_H

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <iostream>

/** The main class for the C++ Vex parser **/
class Vex {
public:
  typedef Vexpp_node                    Node;
  typedef Node::Date                    Date;
  typedef Node::iterator                iterator;
  typedef Node::const_iterator          const_iterator;

  /// Constructor
  Vex();
  Vex(const char* filename);
  bool open(const char* filename);

  // Returns the names of the scans
  template <class OutputIterator>
  OutputIterator get_scans(OutputIterator it) const;

  // Returns the names of the participating stations for a scan
  template <class OutputIterator>
  OutputIterator get_stations(const std::string scan, OutputIterator it) const;

  /// Returns the name of the track section given a mode and a station
  std::string
  get_track(const std::string &mode, const std::string &station) const;
  /// Returns the name of the track section given a mode and a station
  std::string
  get_bitstreams(const std::string &mode, const std::string &station) const;
  /// Returns the name of the frequency section given a mode and a station
  std::string
  get_frequency(const std::string &mode, const std::string &station) const;
  /// Returns the name of the IF section given a mode and a station
  std::string
  get_IF(const std::string &mode, const std::string &station) const;
  /// Returns the name of the BBC section given a mode and a station
  std::string
  get_BBC(const std::string &mode, const std::string &station) const;
  /// Returns the name of a section given the mode and station
  std::string
  get_section(const std::string &section, const std::string &mode, 
              const std::string &station) const;

  /// Returns the mode for a scan
  std::string get_mode(const std::string &scan) const;

  /// Returns the frequencies for a given mode
  void get_frequencies(const std::string &mode, const std::string &station, std::vector<double> &frequencies) const;

  /// Returns the time at which the first scan starts
  std::string get_start_time_of_experiment() const;
  /// Returns the time at which the last scan stops
  std::string get_stop_time_of_experiment() const;
  /// Returns the start time of a given scan
  std::string get_start_time_of_scan(const std::string &scan) const;
  /// Returns the stop time of a given scan
  std::string get_stop_time_of_scan(const std::string &scan) const;
  /// Returns the name of the scan in which the date is located
  std::string get_scan_name(const Vex::Date &start_time) const;
  /// Returns the number of sources in the scan
  int n_sources(const std::string &scan_name) const;

  /// Returns the start time of a given scan
  Date start_of_scan(const std::string &scan) const;
  /// Returns the stop time of a given scan
  Date stop_of_scan(const std::string &scan) const;

  /// Get the position of a station
  void site_position(const std::string &station, double position[]) const;

  /// Returns the polarisation for a given mode, BBC and station
  char polarisation(const std::string &if_node,
                    const std::string &if_ref) const;

  /// Access to the vex data tree:
  const Node &get_root_node() const;
  Node &get_root_node_not_const();
private:
  Node root;
};

// IO operators
std::ostream &operator<<(std::ostream &out, const Vex& vex);

// Template functions
template <class OutputIterator>
OutputIterator
Vex::get_scans(OutputIterator output) const {
  for (Node::const_iterator it = root["SCHED"]->begin();
       it != root["SCHED"]->end(); ++it) {
    *output = it.key();
    output++;
  }
  return output;
}

template <class OutputIterator>
OutputIterator
Vex::get_stations(const std::string scan, OutputIterator output) const {
  Node::const_iterator scan_it = root["SCHED"][scan];
  for (Node::const_iterator it = scan_it->begin("station");
       it != scan_it->end("station"); ++it) {
    *output = it[0]->to_string();
    output++;
  }
  return output;
}

// Access for the python interace
Vex::Node parse_vex(char *filename);

#endif // VEXPP_H
