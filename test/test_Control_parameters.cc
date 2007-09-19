#include <Control_parameters.h>
#include <Log_writer_cout.h>

int main(int argc, char *argv[]) {
  char *ctrl_file = "./data/n06c2.ctrl";
  char *vex_file = "./data/n06c2.vex";
  if (argc == 3) {
    ctrl_file = argv[1];
    vex_file = argv[2];
  }
  
  Log_writer_cout log_writer;
  Control_parameters parameters;
  if (!parameters.initialise(ctrl_file, vex_file, log_writer)) {
    return 1;
  }

  // Get Track_parameters for every scan x station
  std::vector<std::string> scans;
  parameters.get_vex().get_scans(std::back_inserter(scans));
  for (size_t i=0; i<scans.size(); i++) {
    // Get all stations for a certain scan
    std::vector<std::string> stations;
    parameters.get_vex().get_stations(scans[i], std::back_inserter(stations));
    for (size_t j=0; j<stations.size(); j++) {
      const std::string &track = 
        parameters.get_vex().get_track(parameters.get_vex().get_mode(scans[i]),
                                       stations[j]);
      parameters.get_track_parameters(track);
    }
  }

  return 0;
}
