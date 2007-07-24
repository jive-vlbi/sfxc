#include "Control_parameters.h"
#include <fstream>
#include <assert.h>
#include <json/json.h>
#include <vexplus.h>

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
    node = ctrl_root["start"];
    if (!node.isString()) {
      log_writer << "Could not find start in the control file" << std::endl;
      return false;
    }
    start_day = get_day(node.asString().c_str());
    assert(start_day > 0);
    start_time = get_time(node.asString().c_str());
    
    // stop time
    node = ctrl_root["stop"];
    if (!node.isString()) {
      log_writer << "Could not find stop in the control file" << std::endl;
      return false;
    }
    stop_time = get_time(node.asString().c_str());
    
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
      if (is_station(*station)) {
        Station_parameters &station_parameters = get_station(*station);
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
  }    
  return true;
}

bool 
Control_parameters::
parse_vex_file(char *vex_filename, Log_writer& log_writer) {
  // Fill the scan parameters:
  Scan_parameters scan;
  scan.name = "No0005";
  scan.start = get_time("2006y168d07h32m00s");
  if (is_station("Mc")) {
    std::cout << "-------------- Mc" << std::endl;
  }
  char * stations[] = {"Ef", "Wb", "Jb", "On", "Mc", "Ur", "Hh", 
                       "Nt", "Tr", "Sh", "Cm"};
  for (int i=0; i<11; i++) {
    Scan_parameters::Scan_station_parameters scan_station;
    scan_station.station_name = stations[i];
    scan_station.start_second = 0;
    scan_station.stop_second = 600;
    scan_station.frequency = (i<7 ? "4974.49MHz8x8MHz" : "4974.49MHz8x8MHz#02"); 
    scan.stations.push_back(scan_station);
  } 
  scans[scan.start] = scan;
  
  Frequency_parameters frequency;
  frequency.frequency_name = "4974.49MHz8x8MHz";
  frequency.sample_rate = 16000000;
  Frequency_parameters::Frequency_channel_parameters channel;
  channel.channel_name = "CH01";
  channel.frequency = 4974.49;
  channel.sideband = 'U';
  channel.polarisation = 'R';
  channel.band_width = 8000000;
  channel.sign_headstack = 1;
  channel.sign_tracks.push_back(2); channel.sign_tracks.push_back(4);
  channel.magn_headstack = 1;
  channel.magn_tracks.push_back(6); channel.magn_tracks.push_back(8);
  frequency.channels.push_back(channel); 

  channel.channel_name = "CH02";
  channel.polarisation = 'L';
  channel.sign_tracks.clear();
  channel.sign_tracks.push_back(10); channel.sign_tracks.push_back(12);
  channel.magn_tracks.clear();
  channel.magn_tracks.push_back(14); channel.magn_tracks.push_back(16);
  frequency.channels.push_back(channel); 

  frequencies[frequency.frequency_name] = frequency;
  
//  // Vex file
//  VexPlus vex_file(vex_filename);
//  vex_file.parseVex();
//  
//  assert(exper_name == vex_file.ExperName());
//  
////  struct vex *root_node;
////  if (vex_open(vex_filename, &root_node)) {
////    log_writer << "Could not open the vex-file" << std::endl;
////    return false;
////  }
////
////  fp=stdout;
//  
//  if (!set_scan_information(vex_file, log_writer)) {
//    log_writer << "Could not set scan information" << std::endl;
//    return false;
//  }
//  
  return true;
}

bool
Control_parameters::
is_station(const std::string &station) {
  return (stations.find(station) != stations.end());
}

Station_parameters &
Control_parameters::
get_station(const std::string &station) {
  Station_map_it station_it = stations.find(station);
  assert(station_it != stations.end());
  return station_it->second;
}

bool
Control_parameters::
set_scan_information(VexPlus &vex_file, Log_writer &log_writer) {
  int scan=0;
  std::string scanname = vex_file.ScanName(scan);
  while (scanname != "") {
//    std::cout << scanname << std::endl;

    scanname = vex_file.ScanName(scan);
    
    vex_file.ScanStart(scan);
    
    scan++;
  }
  

  return true;
}
  
int
Control_parameters::get_day(const char *time) {
  int year, day, hour, min, sec;
  if (sscanf(time, "%dy%dd%dh%dm%ds", 
             &year, &day, &hour, &min, &sec) != 5) {
    std::cout << "Could not parse start time" << std::endl;
    assert(false);
  }
  // MJD starts at 17 november 1958, midnight
  // there are 51543 days to December, 31st 1999
  day += 51543;
  for (int i=2000; i<year; i++) { 
    day += ((i%4==0) && ((!(i%100==0)) || (i%400==0)) ? 366 : 365);
  }
  return day;
}

int
Control_parameters::get_time(const char *time) {
  int year, day, hour, min, sec;
  if (sscanf(time, "%dy%dd%dh%dm%ds", 
             &year, &day, &hour, &min, &sec) != 5) {
    std::cout << "Could not parse start time" << std::endl;
    assert(false);
  }
  // MJD starts at 17 november 1958, midnight
  // there are 51543 days to December, 31st 1999
  day += 51543;
  for (int i=2000; i<year; i++) { 
    day += ((i%4==0) && ((!(i%100==0)) || (i%400==0)) ? 366 : 365);
  }
  // Time is in miliseconds since 0h00 on the start day:
  return ((((day-start_day)*24 + hour)*60 + min)*60 + sec)*1000;
}

Log_writer &
Control_parameters::operator<<(Log_writer &log_writer) {
  log_writer << "{" << std::endl
             << "\"exper_name\": \"" << exper_name << "\"," << std::endl
             << "\"start_day\": " << start_day << "," << std::endl
             << "\"start_time\": " << start_time << "," << std::endl
             << "\"stop_time\": " << stop_time << "," << std::endl
             << "\"message_level\": " << message_level << "," << std::endl
             << "\"stations\": {" << std::endl;
  for (Station_map_it station_it = stations.begin(); 
       station_it != stations.end();
       station_it++) {
    if (station_it != stations.begin()) log_writer << "," << std::endl;
    log_writer << "  \"" << station_it->first << "\": "
               << station_it->second; 
  }
  log_writer << "}," << std::endl
             << "\"reference_station\": \"" << reference_station << "\"," << std::endl
             << "\"scans\": {" << std::endl;
  for (Scan_map_it scan_it = scans.begin(); 
       scan_it != scans.end();
       scan_it++) {
    if (scan_it != scans.begin()) log_writer << "," << std::endl;
    log_writer << "  \"" << scan_it->first << "\": "
               << scan_it->second; 
  }
  log_writer << "}," << std::endl
             << "\"frequencies\": {" << std::endl;
  for (Frequency_map_it frequency_it = frequencies.begin(); 
       frequency_it != frequencies.end();
       frequency_it++) {
    if (frequency_it != frequencies.begin()) log_writer << "," << std::endl;
    log_writer << "  \"" << frequency_it->first << "\": "
               << frequency_it->second; 
  }
  log_writer << "}," << std::endl 
             << "\"cross_polarize\": " << cross_polarize << "," << std::endl
             << "\"number_channels\": " << number_channels << "," << std::endl
             << "\"integr_time\": " << integr_time << "," << std::endl
             << "\"delay_directory\": \"" << delay_directory << "\"," << std::endl
             << "\"output_file\": \"" << output_file << "\"" << std::endl
             << "}";


  return log_writer;  
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

Log_writer &
Station_parameters::operator<<(Log_writer &log_writer) {
  log_writer << "{ \"data_sources\": [";
  for (std::list<std::string>::iterator data_source = data_sources.begin();
       data_source != data_sources.end();
       data_source++) {
    if (data_source != data_sources.begin()) log_writer << ", ";
    log_writer << "\"" << *data_source << "\"";
  } 
  log_writer << "] }";
  return log_writer;
}

/**********************************************/
/** Scan_parameters                          **/
/**********************************************/

Log_writer &
Scan_parameters::operator<<(Log_writer &log_writer) {
  log_writer << "{ \"name\": \"" << name << "\"," << std::endl
             << "\"start\": " << start << "," << std::endl
             << "\"stations\": [";
  for (std::list<Scan_station_parameters>::iterator station = stations.begin();
       station != stations.end();
       station++) {
    if (station != stations.begin()) log_writer << ", " << std::endl;
    log_writer << *station;
  } 
  log_writer << "] }" << std::endl;
  return log_writer;
}

/**********************************************/
/** Scan_parameters::Scan_station_parameters **/
/**********************************************/
Log_writer &
Scan_parameters::Scan_station_parameters::operator<<(Log_writer &log_writer) {
  log_writer << "{ \"station_name\": \"" << station_name << "\"," << std::endl
             << "\"frequency\": \"" << frequency << "\", " << std::endl
             << "\"start_second\": " << start_second << ", " << std::endl
             << "\"stop_second\": " << stop_second << " }"; 
  return log_writer;
}

/**********************************************/
/** Frequency_parameters                     **/
/**********************************************/
Log_writer &
Frequency_parameters::operator<<(Log_writer &log_writer) {
  log_writer << "{ \"frequency_name\": \"" << frequency_name << "\","
             << std::endl
             << "\"sample_rate\": " << sample_rate << "," << std::endl
             << "\"channels\": [";
  for (std::list<Frequency_channel_parameters>::iterator ch = channels.begin();
       ch != channels.end();
       ch++) {
    if (ch != channels.begin()) log_writer << ", " << std::endl;
    log_writer << *ch;
  }
  log_writer << "] }" << std::endl;
  return log_writer;
}

Log_writer &
Frequency_parameters::Frequency_channel_parameters::
operator<<(Log_writer &log_writer) {
  log_writer << "{ " << std::endl
             << "\"channel_name\": \"" << channel_name << "\", " << std::endl
             << "\"frequency\": " << frequency << ", " << std::endl
             << "\"sideband\": \"" << sideband << "\", " << std::endl
             << "\"polarisation\": \"" << polarisation << "\", " << std::endl
             << "\"band_width\": " << band_width << ", " << std::endl
             << "\"sign_headstack\": " << sign_headstack << ", " << std::endl
             << "\"sign_tracks\": [";
  for (std::list<int>::iterator tracks = sign_tracks.begin();
       tracks != sign_tracks.end(); tracks++) {
    if (tracks != sign_tracks.begin()) log_writer << ", ";
    log_writer << *tracks;
  }
  log_writer << "], " << std::endl
           << "\"magn_headstack\": " << magn_headstack << ", " << std::endl
           << "\"magn_tracks\": [";
  for (std::list<int>::iterator tracks = magn_tracks.begin();
       tracks != magn_tracks.end(); tracks++) {
    if (tracks != magn_tracks.begin()) log_writer << ", ";
    log_writer << *tracks;
  }
  log_writer << "] }";
             
  return log_writer;
}

/**********************************************/
/** IO operators                             **/
/**********************************************/
Log_writer &operator<<(Log_writer &log_writer, 
                       Control_parameters &parameters) {
  return parameters.operator<<(log_writer);
}
Log_writer &operator<<(Log_writer &log_writer, 
                       Station_parameters &station) {
  return station.operator<<(log_writer);
}
Log_writer &operator<<(Log_writer &log_writer, 
                       Scan_parameters &scan) {
  return scan.operator<<(log_writer);
}
Log_writer &operator<<(Log_writer &log_writer, 
                       Scan_parameters::Scan_station_parameters &scan) {
  return scan.operator<<(log_writer);
}
Log_writer &operator<<(Log_writer &log_writer, Frequency_parameters &freq) {
  return freq.operator<<(log_writer);
}
Log_writer &
operator<<(Log_writer &log_writer, 
           Frequency_parameters::Frequency_channel_parameters &freq_ch) {
  return freq_ch.operator<<(log_writer);
}
