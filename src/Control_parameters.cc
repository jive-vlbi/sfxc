#include "Control_parameters.h"
#include <fstream>
#include <assert.h>
#include <json/json.h>

#include <utils.h>

Control_parameters::Control_parameters()
 : initialised(false) {
}

Control_parameters::Control_parameters(const char *ctrl_file, 
                                       const char *vex_file, 
                                       std::ostream& log_writer)
 : initialised(false) {
  initialise(ctrl_file, vex_file, log_writer);
}

bool 
Control_parameters::
initialise(const char *ctrl_filename, const char *vex_filename, 
           std::ostream& log_writer) {
  { // parse the control file
    Json::Reader reader;
    std::ifstream in(ctrl_filename);
    if (!in.is_open()) {
      log_writer << "Could not open control file" << std::endl;
      return false;
    }
    bool ok = reader.parse(in, ctrl);
    if ( !ok ) {
      // report to the user the failure and their locations in the document.
      log_writer  << "Failed to parse configuration\n"
                  << reader.getFormatedErrorMessages()
                  << std::endl;
      return false;
    }
  }

  { // parse the vex file
    if (!vex.open(vex_filename)) return false;
  }

  initialised = true;

  return true;
}

Control_parameters::Date 
Control_parameters::get_start_time() {
  return Date(ctrl["start"].asString());
}

Control_parameters::Date 
Control_parameters::get_stop_time() {
  return Date(ctrl["stop"].asString());
}

std::vector<std::string> 
Control_parameters::data_sources(const std::string &station) const {
  std::vector<std::string> result;
  Json::Value data_sources = ctrl["data_sources"][station];
  assert(data_sources != Json::Value());
  for (size_t index = 0; 
       index < ctrl["data_sources"][station].size(); ++index ) {
    result.push_back(ctrl["data_sources"][station][index].asString());
  }
  return result;
}

std::string 
Control_parameters::station(int i) const {
  return ctrl["stations"][i].asString();
}

size_t
Control_parameters::number_stations() const {
  return ctrl["stations"].size();
}



const Vex &
Control_parameters::get_vex() const {
  assert(initialised);
  return vex;
}




Track_parameters 
Control_parameters::get_track_parameters(const std::string &track_name) const {
  Track_parameters result;
  result.sample_rate = -1;
  Vex::Node::const_iterator track = vex.get_root_node()["TRACKS"][track_name];

  for (Vex::Node::const_iterator fanout_def_it = track->begin("fanout_def");
       fanout_def_it != track->end("fanout_def"); ++fanout_def_it) {
    // sample_rate
    for (Vex::Node::const_iterator freq = vex.get_root_node()["FREQ"]->begin();
         freq != vex.get_root_node()["FREQ"]->end(); ++freq) {
      for (Vex::Node::const_iterator chan = freq->begin("chan_def");
           chan != freq->end("chan_def"); ++chan) {
        if (chan[4]->to_string() == fanout_def_it[1]->to_string()) {
          if (result.sample_rate == -1) {
            result.sample_rate = 
              (int)(freq["sample_rate"]->to_double_amount("Ms/sec"));
          } else {
            assert(result.sample_rate ==
                   (int)(freq["sample_rate"]->to_double_amount("Ms/sec")));
          }
        }
      }
    }

    // tracks
    Track_parameters::Channel_parameters &channel_param
      = result.channels[fanout_def_it[1]->to_string()];
    Vex::Node::const_iterator it = fanout_def_it->begin();
    ++it; ++it; ++it;
    if (fanout_def_it[2]->to_string() == "sign") {
      channel_param.sign_headstack = it->to_int();
      ++it;
      for (; it != fanout_def_it->end(); ++it) {
        channel_param.sign_tracks.push_back(it->to_int());
      }
    } else {
      assert(fanout_def_it[2]->to_string() == "mag");
      channel_param.magn_headstack = it->to_int();
      ++it;
      for (; it != fanout_def_it->end(); ++it) {
        channel_param.magn_tracks.push_back(it->to_int());
      }
    }
  }

  return result;
}

Correlation_parameters 
Control_parameters::
get_correlation_parameters(const std::string &scan_name) const {
  assert(false);
}

std::string
Control_parameters::
get_delay_table_name(const std::string &station_name) const {
  assert(false);
  return std::string("");
}



int 
Track_parameters::Channel_parameters::bits_per_sample() const {
  return (magn_tracks.size() == 0 ? 1 : 2);
}


