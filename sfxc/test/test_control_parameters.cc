#include "utils.h"
#include "control_parameters.h"
#include "log_writer_cout.h"

int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  char *ctrl_file = "./data/n06c2.ctrl";
  char *vex_file = "./data/n06c2.vex";
  if (argc == 3) {
    ctrl_file = argv[1];
    vex_file = argv[2];
  }

  Log_writer_cout log_writer;
  Control_parameters control_parameters;
  if (!control_parameters.initialise(ctrl_file, vex_file, log_writer)) {
    return 1;
  }

  // Get Track_parameters for every scan x station
  std::vector<std::string> scans;
  control_parameters.get_vex().get_scans(std::back_inserter(scans));

  for (size_t i=0; i<scans.size(); i++) {
    // Get all stations for a certain scan
    std::vector<std::string> stations;
    control_parameters.get_vex().get_stations(scans[i], std::back_inserter(stations));
    for (size_t j=0; j<stations.size(); j++) {
      const std::string &mode =
        control_parameters.get_vex().get_mode(scans[i]);
      control_parameters.get_input_node_parameters(mode, stations[j]);
    }
  }

  // Get station map:
  std::map<std::string, int> station_streams;
  for (size_t station_nr=0;
       station_nr<control_parameters.number_stations();
       station_nr++) {
    const std::string &station_name =
      control_parameters.station(station_nr);
    station_streams[station_name] = station_nr;
  }

  for (size_t i=0; i<scans.size(); i++) {
    const Vex::Node &root_node =
      control_parameters.get_vex().get_root_node();
    std::string mode = root_node["SCHED"][scans[i]]["mode"]->to_string();
    std::string freq = root_node["MODE"][mode]["FREQ"][0]->to_string();

    for (Vex::Node::const_iterator
         station = root_node["FREQ"][freq]->begin("chan_def");
         station != root_node["FREQ"][freq]->end("chan_def"); ++station) {
      // Check the correlation parameters
      std::vector<std::string> station_name;
      station_name.push_back("Wb");
      control_parameters.get_correlation_parameters(scans[i],
          station[4]->to_string(),
          station_name,
          station_streams);
    }
  }
  return 0;
}
