#include "Control_parameters.h"
#include <fstream>
#include <assert.h>
#include "json/json.h"
#include "vexplus.h"

Control_parameters::Control_parameters()
 : reference_station(""), initialised(false) {
}

bool 
Control_parameters::
initialise(char *ctrl_file, char *vex_file, Log_writer& log_writer) {
  if (!parse_control_file(ctrl_file, log_writer)) {
    return false;
  }
  if (!parse_vex_file(vex_file, log_writer)) {
    return false;
  }

  initialised = true;
  return true;
}

bool 
Control_parameters::
parse_control_file(char *ctrl_filename, Log_writer& log_writer) {
  Json::Value ctrl_root, node;
  { // parse the control file
    Json::Reader reader;
    std::ifstream in(ctrl_filename);
    if (!in.is_open()) {
      log_writer << "Could not open control file" << std::endl;
      return false;
    }
    bool ok = reader.parse(in, ctrl_root);
    if ( !ok ) {
      // report to the user the failure and their locations in the document.
      log_writer  << "Failed to parse configuration\n"
                  << reader.getFormatedErrorMessages()
                  << std::endl;
      return false;
    }
  }
  
  // extract the data from the json-datastructure
  
  node = ctrl_root["exper_name"];
  if (!node.isString()) {
    log_writer << "Could not find exper_name in the control file" << std::endl;
    return false;
  } 
  exper_name = node.asString();

  { //start and stop time
    int year, day, hour, min, sec;  
    node = ctrl_root["start"];
    if (!node.isString()) {
      log_writer << "Could not find start in the control file" << std::endl;
      return false;
    }
    
    if (sscanf(node.asString().c_str(), "%dy%dd%dh%dm%ds", 
               &year, &day, &hour, &min, &sec) != 5) {
      log_writer << "Could not parse start time" << std::endl;
      return false;
    }

    // MJD starts at 17 november 1958
    // there are 51543 days to December, 31st 1999
    start_day = 51543 + day;
    for (int i=2000; i<year; i++) { 
      start_day += ((i%4==0) && ((!(i%100==0)) || (i%400==0)) ? 366 : 365);
    }
    start_time = (hour*60 + min)*60 + sec;

    // stop time
    node = ctrl_root["stop"];
    if (!node.isString()) {
      log_writer << "Could not find stop in the control file" << std::endl;
      return false;
    }
    
    if (sscanf(node.asString().c_str(), "%dy%dd%dh%dm%ds", 
               &year, &day, &hour, &min, &sec) != 5) {
      log_writer << "Could not parse stop time" << std::endl;
      return false;
    }

    // MJD starts at 17 november 1958
    // there are 51543 days to December, 31st 1999
    int stop_day = 51543 + day;
    for (int i=2000; i<year; i++) { 
      stop_day += ((i%4==0) && ((!(i%100==0)) || (i%400==0)) ? 366 : 365);
    }
    stop_time = ((stop_day-start_day)*24 + (hour*60 + min))*60 + sec;
    
    if (stop_time <= start_time) {
      log_writer << "Start time should be before stop time" << std::endl;
      return false;
    }
  }

  { // Station section
    node = ctrl_root["stations"];
    if (!node.isArray()) {
      log_writer << "The stations field is not an array" << std::endl;
      return false;
    }
    for (unsigned int i=0; i<node.size(); i++) {
      if (!node[i].isString()) {
        log_writer << "Element " << i << " of the stations array is not a string"
                   << std::endl;
        return false;
      }
      // Insert the station to the list of stations
      stations[node[i].asString()];
    }
  
    if (ctrl_root.isMember("reference_station")) {
      if (!ctrl_root["reference_station"].isString()) {
        log_writer << "Reference station is not a string" << std::endl;
        return false;
      }
      if (stations.find(ctrl_root["reference_station"].asString()) ==
          stations.end()) {
        log_writer << "Reference stations is not one of the stations"
                   << std::endl;
        return false;
      }
      reference_station = ctrl_root["reference_station"].asString();
    }
  }
  
  { // Parameters related to the correlation
    node = ctrl_root["cross_polarize"];
    if (!node.isBool()) {
      log_writer << "Cross polarize not defined" << std::endl;
      return false;
    }
    cross_polarize = node.asBool();

    node = ctrl_root["number_channels"];
    if (!node.isInt()) {
      log_writer << "Number of channels is not defined" << std::endl;
      return false;
    }
    number_channels = node.asInt();
  
    node = ctrl_root["integr_time"];
    if (!node.isDouble()) {
      log_writer << "Integration time is not defined" << std::endl;
      return false;
    }
    integr_time = node.asDouble();

    node = ctrl_root["message_level"];
    if (!node.isInt()) {
      log_writer << "Message level is not defined" << std::endl;
      return false;
    }
    message_level = node.asInt();
  }

  { // Data streams
    node = ctrl_root["delay_directory"];
    if (!node.isString()) {
      log_writer << "Delay directory is not defined" << std::endl;
      log_writer << "NGHK: This still has to be implemented" << std::endl;
      return false;
    }
    delay_directory = node.asString();

    node = ctrl_root["output_file"];
    if (!node.isString()) {
      log_writer << "Output file is not defined" << std::endl;
      return false;
    }
    output_file = node.asString();

    node = ctrl_root["data_sources"];
    if (!node.isObject()) {
      log_writer << "Data sources are not defined" << std::endl;
      return false;
    }
    
    Json::Value::Members station_names = node.getMemberNames();
    for (Json::Value::Members::iterator station = station_names.begin();
         station != station_names.end();
         station++) {
      Station_parameters station_parameters = get_station(*station);
      Json::Value station_sources = node[*station];
      // add data sources
      for (Json::Value::iterator source = station_sources.begin();
           source != station_sources.end();
           source++) {
        if (!(*source).isString()) {
          log_writer << "Data source is not a string" << std::endl;
          return false;
        }
        station_parameters.add_data_source((*source).asString());
      }
    }
  }    
  return true;
}

bool 
Control_parameters::
parse_vex_file(char *vex_filename, Log_writer& log_writer) {
  VexPlus vex_file(vex_filename);
  vex_file.parseVex();

  return true;
}

Station_parameters &
Control_parameters::
get_station(const std::string &station) {
  Station_map_it station_it = stations.find(station);
  assert(station_it != stations.end());
  return station_it->second;
}

/**********************************************/
/** Station_parameters                       **/
/**********************************************/
Station_parameters::Station_parameters() {
}

void 
Station_parameters::add_data_source(const std::string &source) {
  data_sources.push_back(source);
}
