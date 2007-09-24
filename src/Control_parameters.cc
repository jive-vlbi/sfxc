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

int
Control_parameters::integration_time() const {
  return (int)(1000*ctrl["integr_time"].asDouble());
}

int
Control_parameters::number_channels() const {
  return ctrl["number_channels"].asInt();
}

int
Control_parameters::bits_per_sample() const {
  Vex::Node::const_iterator track = vex.get_root_node()["TRACKS"]->begin();
  int bits = 1;
  for (Vex::Node::const_iterator fanout_def_it = track->begin("fanout_def");
       fanout_def_it != track->end("fanout_def"); ++fanout_def_it) {
    if (fanout_def_it[2]->to_string() == "mag") {
      bits = 2;
    }
  }
  
  // NGHK: still a hack, assumes all samples use the same number of bits
  // Checking the precondition here
  for (track = vex.get_root_node()["TRACKS"]->begin();
       track != vex.get_root_node()["TRACKS"]->end(); ++track) {
    std::map<std::string, int> result;
    // set all channels to zero:
    for (Vex::Node::const_iterator fanout_def_it = track->begin("fanout_def");
         fanout_def_it != track->end("fanout_def"); ++fanout_def_it) {
      result[fanout_def_it[1]->to_string()] = 0;
    }
    // Count the number of bits
    for (Vex::Node::const_iterator fanout_def_it = track->begin("fanout_def");
         fanout_def_it != track->end("fanout_def"); ++fanout_def_it) {
      result[fanout_def_it[1]->to_string()] += 1;
    }

    for (std::map<std::string, int>::iterator it = result.begin();
         it != result.end(); it++) {
      assert(it->second == bits);
    }
  }
  
  return bits;
}

std::string
Control_parameters::scan(int scan_nr) const {
  Vex::Node::const_iterator it = vex.get_root_node()["SCHED"]->begin();
  for (int curr=0; curr < scan_nr; ++curr) {
    ++it;
    assert(it != vex.get_root_node()["SCHED"]->end());
  }
  return it.key();
}

size_t
Control_parameters::number_scans() const {
  return vex.get_root_node()["SCHED"]->size();
}

std::string 
Control_parameters::
station_in_scan(const std::string& scan, int station_nr) const {
  Vex::Node::const_iterator it = 
    vex.get_root_node()["SCHED"][scan]->begin("station");
  for (int curr=0; curr < station_nr; curr++) {
    ++it;
    assert(it != vex.get_root_node()["SCHED"][scan]->end("station"));
  }
  return it[0]->to_string();
}

size_t 
Control_parameters::number_stations_in_scan(const std::string& scan) const {
  size_t n_scans=0;
  for (Vex::Node::const_iterator it = 
         vex.get_root_node()["SCHED"][scan]->begin("station");
       it != vex.get_root_node()["SCHED"][scan]->end("station");
       ++it) {
    n_scans++;
  }
  return n_scans;
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
    const std::string &channel_name = fanout_def_it[1]->to_string();
    // sample_rate
    for (Vex::Node::const_iterator freq = vex.get_root_node()["FREQ"]->begin();
         freq != vex.get_root_node()["FREQ"]->end(); ++freq) {
      for (Vex::Node::const_iterator chan = freq->begin("chan_def");
           chan != freq->end("chan_def"); ++chan) {
        if (chan[4]->to_string() == channel_name) {
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
get_correlation_parameters(const std::string &scan_name,
                           const std::map<std::string, int> 
                           &correlator_node_station_to_input) const {
  Vex::Node::const_iterator scan = 
    vex.get_root_node()["SCHED"][scan_name];
  Vex::Node::const_iterator mode = 
    vex.get_root_node()["MODE"][scan["mode"]->to_string()];

  Correlation_parameters corr_param;
  corr_param.start_time = vex.start_of_scan(scan_name).to_miliseconds();
  corr_param.stop_time = vex.stop_of_scan(scan_name).to_miliseconds();
  corr_param.integration_time = integration_time();
  corr_param.number_channels = number_channels();

  // Assumption: sample rate the the same for all stations:
  Vex::Node::const_iterator freq = 
    vex.get_root_node()["FREQ"][mode["FREQ"][0]->to_string()];
  corr_param.sample_rate =
    (int)(1000000*freq["sample_rate"]->to_double_amount("Ms/sec"));
  
  // assumes the same bits per sample for all stations
  corr_param.bits_per_sample = bits_per_sample();

  // now get the station streams
  for (Vex::Node::const_iterator station = scan->begin("station");
       station != scan->end("station"); ++station) {
    std::map<std::string, int>::const_iterator station_nr_it =
      correlator_node_station_to_input.find(station[0]->to_string());
    if (station_nr_it != correlator_node_station_to_input.end()) {
      if (station_nr_it->second >= 0) {
        Correlation_parameters::Station_parameters station_param;
        station_param.station_stream = station_nr_it->second;
        station_param.start_time = station[1]->to_int_amount("sec");
        station_param.stop_time = station[2]->to_int_amount("sec");
        corr_param.station_streams.push_back(station_param);
      }
    }
  }

  return corr_param;
}

std::string
Control_parameters::
get_delay_table_name(const std::string &station_name) const {
  assert(false);
  return std::string("");
}

bool 
Track_parameters::operator==(const Track_parameters &other) const {
  if (sample_rate != other.sample_rate) return false;
  if (channels != other.channels) return false;

  return true;
}

int 
Track_parameters::Channel_parameters::bits_per_sample() const {
  return (magn_tracks.size() == 0 ? 1 : 2);
}


bool 
Track_parameters::Channel_parameters::
operator==(const Track_parameters::Channel_parameters &other) const {
  if (sign_headstack != other.sign_headstack) return false;
  if (magn_headstack != other.magn_headstack) return false;
  if (sign_tracks != other.sign_tracks) return false;
  if (magn_tracks != other.magn_tracks) return false;
  
  return true;
}

bool 
Correlation_parameters::operator==(const Correlation_parameters& other) const {
  if (start_time != other.start_time) return false;
  if (stop_time != other.stop_time) return false;
  if (integration_time != other.integration_time) return false;
  if (number_channels != other.number_channels) return false;
  if (sample_rate != other.sample_rate) return false;
  if (bits_per_sample != other.bits_per_sample) return false;
  if (station_streams != station_streams) return false;
  return true;
}

bool 
Correlation_parameters::Station_parameters::
operator==(const Correlation_parameters::Station_parameters& other) const {
  if (station_stream != other.station_stream) return false;
  if (start_time != other.start_time) return false;
  if (stop_time != other.stop_time) return false;

  return true;
}
