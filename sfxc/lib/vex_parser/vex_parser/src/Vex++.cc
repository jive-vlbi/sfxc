#include <assert.h>
#include <algorithm>

#include "vex/Vex++.h"
#include "generic.h"
#include <set>

/**
 *  Vex : Wrapper around the vex data tree
 **/
Vex::Vex() {}

Vex::Vex(const char* filename) {
  int result = open(filename);
  assert(result);
}
bool Vex::open(const char* filename) {
  yyin = fopen( filename, "r" );
  assert(yyin != NULL);

  int result = yyparse();
  root = parse_result;

  return (result==0);
}

const Vex::Node &
Vex::get_root_node() const {
  return root;
}

Vex::Node &
Vex::get_root_node_not_const() {
  return root;
}

std::string
Vex::get_mode(const std::string &scan) const {
  return root["SCHED"][scan]["mode"]->to_string();
}


std::string
Vex::get_start_time_of_experiment() const {
  std::string result = "";
  Vex::Node::const_iterator sched = root["SCHED"];
  for (Vex::Node::const_iterator scan_it = sched->begin();
       scan_it != sched->end(); ++scan_it) {
    if (result == "") {
      result = scan_it["start"]->to_string();
    } else if (result > scan_it["start"]->to_string()) {
      result = scan_it["start"]->to_string();
    }
  }
  return result;
}

std::string
Vex::get_stop_time_of_experiment() const {
  std::string result = "";
  Vex::Node::const_iterator sched = get_root_node()["SCHED"];
  for (Vex::Node::const_iterator scan_it = sched->begin();
       scan_it != sched->end(); ++scan_it) {
    std::string curr = get_stop_time_of_scan(scan_it.key());
    if (result == "") {
      result = curr;
    } else if (result < curr) {
      result = curr;
    }
  }
  return result;
}

std::string
Vex::get_start_time_of_scan(const std::string &scan_name) const {
  Vex::Node::const_iterator scan = root["SCHED"][scan_name];
  std::string start = scan["start"]->to_string();
  return start;
}

std::string
Vex::get_stop_time_of_scan(const std::string &scan_name) const {
  Vex::Node::const_iterator scan = root["SCHED"][scan_name];

  std::string start = scan["start"]->to_string();

  Date duration;
  for (Vex::Node::const_iterator it = scan->begin("station");
       it != scan->end("station"); ++it) {
    int curr_duration;
    sscanf(it[2]->to_string().c_str(),"%d sec", &curr_duration);
    if (curr_duration > duration.second) duration.second = curr_duration;
  }
  return (Date(start) + duration).to_string();
}

Vex::Date
Vex::start_of_scan(const std::string &scan_name) const {
  return Date(get_start_time_of_scan(scan_name));
}

Vex::Date
Vex::stop_of_scan(const std::string &scan_name) const {
  return Date(get_stop_time_of_scan(scan_name));
}

std::string
Vex::get_scan_name(const Vex::Date &start_time) const{
  std::string mode;
  for(Vex::Node::const_iterator it = root["SCHED"]->begin();it!=root["SCHED"]->end();it++){
    if((start_time>=start_of_scan(it.key()))&&(start_time<stop_of_scan(it.key()))){
      return it.key();
    }
 }
 return std::string();
}

std::string
Vex::get_track(const std::string &mode, const std::string &station) const {
  return get_section("TRACKS", mode, station);
}

std::string
Vex::get_bitstreams(const std::string &mode, const std::string &station) const {
  return get_section("BITSTREAMS", mode, station);
}

std::string
Vex::get_frequency(const std::string &mode, const std::string &station) const {
  return get_section("FREQ", mode, station);
}

std::string
Vex::get_IF(const std::string &mode, const std::string &station) const {
  return get_section("IF", mode, station);
}

std::string
Vex::get_BBC(const std::string &mode, const std::string &station) const {
  return get_section("BBC", mode, station);
}

std::string
Vex::get_section(const std::string &section, const std::string &mode,
                 const std::string &station) const {
  // Given a mode and a station find the start node (section is e.g. IF, BBC, FREQ, ...)
  Vex::Node::const_iterator mode_it = root["MODE"][mode];
  for (Vex::Node::const_iterator section_it = mode_it->begin(section);
       section_it != mode_it->end(section); ++section_it) {
    Vex::Node::const_iterator station_it = section_it->begin();
    ++station_it;
    for (;station_it != section_it->end(); ++station_it) {
      if (station_it->to_string() == station) {
        return section_it[0]->to_string();
      }
    }
  }
  return std::string();
}

void Vex::site_position(const std::string &station, double position[]) const {
  const std::string &site = root["STATION"][station]["SITE"]->to_string();
  Vex::Node::const_iterator pos_node = root["SITE"][site]["site_position"];
  int i=0;
  for (Vex::Node::const_iterator site = pos_node->begin();
       site != pos_node->end(); ++site) {
    double pos;
    int err = sscanf(site->to_string().c_str(), "%lf m", &pos);
    assert(err == 1);
    position[i] = pos;
    i++;
  }
  assert(i==3);
}

char
Vex::polarisation(const std::string &if_node,
                  const std::string &if_ref) const {
  for (Vex::Node::const_iterator if_it = get_root_node()["IF"][if_node]->begin();
       if_it != get_root_node()["IF"][if_node]->end(); ++if_it) {
    if (if_it[0]->to_string() == if_ref) {
      return if_it[2]->to_char();
    }
  }

  return ' ';
}

/* Global functions */
std::ostream &operator<<(std::ostream &out, const Vex& vex) {
  vex.get_root_node().print(out, 0);
  out << std::endl;
  return out;
}

// For the python interface:
Vex::Node parse_vex(char *filename) {
  yyin = fopen( filename, "r" );
  if (yyin == NULL) {
    std::cout << "Could not open vexfile" << std::endl;
    return Vex::Node();
  }
  yyrestart( yyin );

  int result = yyparse();
  if (result != 0) {
    std::cout << "Could not parse vexfile" << std::endl;
    return Vex::Node();
  }

  return parse_result;
}

int Vex::n_sources(const std::string &scan_name) const{
  int n = 0;
  Vex::Node::const_iterator scan = root["SCHED"][scan_name];
  for (Vex::Node::const_iterator source_it = scan->begin("source");
       source_it != scan->end("source"); ++source_it){
    n++;
  }

  return n;
}

void Vex::get_frequencies(const std::string &mode, const std::string &station, std::vector<double> &frequencies) const {
  Node root_node = get_root_node();
  std::string freq_node = get_frequency(mode, station);
  std::set<double> freq_set;
  for (Node::iterator chan_it = root_node["FREQ"][freq_node]->begin("chan_def");
       chan_it != root_node["FREQ"][freq_node]->end("chan_def"); ++chan_it) {
    freq_set.insert((*chan_it)[1]->to_double_amount("MHz")*1000000);
  }

  frequencies.resize(0);
  for (std::set<double>::iterator freq_it = freq_set.begin();
       freq_it != freq_set.end(); freq_it++) {
    frequencies.push_back(*freq_it);
  }
}
