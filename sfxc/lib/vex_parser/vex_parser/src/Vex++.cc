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
Vex::get_scan_name(Vex::Date &start_time) const{
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
  Vex::Node::const_iterator mode_it = root["MODE"][mode];
  for (Vex::Node::const_iterator track_it = mode_it->begin("TRACKS");
       track_it != mode_it->end("TRACKS"); ++track_it) {
    Vex::Node::const_iterator station_it = track_it->begin();
    ++station_it;
    for (;station_it != track_it->end(); ++station_it) {
      if (station_it->to_string() == station) {
        return track_it[0]->to_string();
      }
    }
  }
  assert(false);
  return std::string();
}

std::string
Vex::get_frequency(const std::string &mode, const std::string &station) const {
  return get_block_node("FREQ", mode, station);
}

std::string
Vex::get_IF_node(const std::string &mode, const std::string &station) const {
  return get_block_node("IF", mode, station);
}

std::string
Vex::get_BBC_node(const std::string &mode, const std::string &station) const {
  return get_block_node("BBC", mode, station);
}

std::string
Vex::get_block_node(const std::string &block, const std::string &mode,
                      const std::string &station) const {
  // Given a mode and a station find the start node (block is e.g. IF, BBC, FREQ, ...)
  Vex::Node::const_iterator mode_it = root["MODE"][mode];
  for (Vex::Node::const_iterator frequency_it = mode_it->begin(block);
       frequency_it != mode_it->end(block); ++frequency_it) {
    Vex::Node::const_iterator station_it = frequency_it->begin();
    ++station_it;
    for (;station_it != frequency_it->end(); ++station_it) {
      if (station_it->to_string() == station) {
        return frequency_it[0]->to_string();
      }
    }
  }
  assert(false);
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

void Vex::get_frequencies(const std::string &mode, std::vector<double> &frequencies) const {
  Node root_node = get_root_node();
  Node::iterator mode_it = root_node["MODE"][mode];
  std::string freq_node = mode_it->begin("FREQ")[0]->to_string();
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
