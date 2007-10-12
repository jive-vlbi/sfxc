#include <Vex++.h>

#include <assert.h>
#include <string>
#include <stdio.h>
#include <fstream>
#include <set>

#include <json/json.h>

std::string add_time(std::string &time, int delta) {
  int year, day, hour, minute, second;
  int n_matched = sscanf(time.c_str(), "%dy%dd%dh%dm%ds", 
                         &year, &day, &hour, &minute, &second);
  assert(n_matched == 5);
  second += delta;
  minute += second/60;
  hour += minute/60;
  day  += hour/24;
  int days_per_year = 365;
  if ((year % 4 == 0) && ((year % 100 != 0) && (year % 400 == 0))) {
    days_per_year = 366;
  }
  year += day/days_per_year;
  char result[20];
  snprintf(result, 20, "%04dy%03dd%02dh%02dm%02ds",
           year, (day%days_per_year), (hour%24), (minute%60), (second%60));
  return std::string(result);
}

std::string get_start(const Vexpp_node &vex) {
  std::string result = "";
  Vex::Node::const_iterator sched = vex["SCHED"];
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

std::string get_endtime_of_scan(const Vexpp_node &scan) {
  std::string start = scan["start"]->to_string();
  int duration = 0;
  for (Vex::Node::const_iterator it = scan.begin("station");
       it != scan.end("station"); ++it) {
    int curr_duration;
    sscanf(it[2]->to_string().c_str(),"%d sec", &curr_duration);
    duration = (curr_duration > duration ? curr_duration : duration);
  }
  return add_time(start, duration);
}

std::string get_stop(const Vexpp_node &vex) {
  std::string result = "";
  Vex::Node::const_iterator sched = vex["SCHED"];
  for (Vex::Node::const_iterator scan_it = sched->begin();
       scan_it != sched->end(); ++scan_it) {
    std::string curr = get_endtime_of_scan(*scan_it);
    if (result == "") {
      result = curr;
    } else if (result < curr) {
      result = curr;
    }
  }
  return result;
}

Json::Value get_frequency(const Vex &vex,
                          Vexpp_node::const_iterator channel,
                          const std::string &ref_station, 
                          const std::string &BBC_block,
                          const std::string &IF_block) {
  Json::Value result;
  std::string if_ref;
  for (Vex::Node::const_iterator bbc = 
         vex.get_root_node()["BBC"][BBC_block]->begin("BBC_assign");
       bbc != vex.get_root_node()["BBC"][BBC_block]->end("BBC_assign"); ++bbc) {
    if (bbc[0]->to_string() == channel[5]->to_string()) {
      if_ref = bbc[2]->to_string();
    }
  }

  double freq, bandwidth;
  int err = sscanf(channel[1]->to_string().c_str(), "%lf MHz", &freq);
  assert(err == 1);
  err = sscanf(channel[3]->to_string().c_str(), "%lf MHz", &bandwidth);
  assert(err == 1);
  
  result["frequency"] = freq*1000000;
  result["bandwidth"] = bandwidth*1000000;
  result["sideband"]  = channel[2]->to_string()+"SB";
  result["polarisation"]  = std::string(1,vex.polarisation(IF_block, if_ref));
  return result;
}

Json::Value get_frequencies(const Vex &vex) {
  // Find the right BBC and IF
  std::string mode = vex.get_root_node()["MODE"]->begin().key();
  std::string IF_block = vex.get_root_node()["MODE"][mode]["IF"][0]->to_string();
  std::string ref_station = vex.get_root_node()["MODE"][mode]["IF"][1]->to_string();
  std::string BBC_block = "";
  // Find the corresponding BBC block:
  for (Vex::Node::const_iterator bbc = vex.get_root_node()["MODE"][mode]->begin("BBC");
       bbc != vex.get_root_node()["MODE"][mode]->end("BBC"); ++bbc) {
    for (Vex::Node::const_iterator station = ++(bbc->begin()); 
	 station != (*bbc).end(); ++station) {
      if (ref_station == station->to_string()) {
	BBC_block = bbc[0]->to_string();
      }
    }
  }

  Json::Value result;
  for (Vex::Node::const_iterator channel = vex.get_root_node()["FREQ"]->begin()->begin("chan_def");
       channel != vex.get_root_node()["FREQ"]->begin()->end("chan_def"); ++channel) {
    result.append(get_frequency(vex, channel, ref_station, BBC_block, IF_block));
  }
//   result = []
//   for freq in vex["FREQ"]:
//     for channel in vex["FREQ"][freq].getall("chan_def"):
//       result.append(get_frequency(IF, BBC, channel))
//     break
  return result;
}

Json::Value site_position(const Vexpp_node &vex,
			  const std::string &station) {
  Json::Value result;

  Vex::Node::const_iterator position = 
    vex["SITE"][vex["STATION"][station]["SITE"]->to_string()]["site_position"];
  for (Vex::Node::const_iterator site = position->begin();
       site != position->end(); ++site) {
    double pos;
    int err = sscanf(site->to_string().c_str(), "%lf m", &pos);
    assert(err == 1);
    result.append(pos);
  }
  return result;
}

Json::Value get_channels(const Vex &vex){
	std::set<std::string> result_set;
	Json::Value result;
	
  for (Vex::Node::const_iterator frq_block = vex.get_root_node()["FREQ"]->begin();
       frq_block != vex.get_root_node()["FREQ"]->end(); ++frq_block) {
  	for (Vex::Node::const_iterator freq_it = frq_block->begin("chan_def");
         freq_it != frq_block->end("chan_def"); ++freq_it) {
			result_set.insert(freq_it[4]->to_string());
		}
	}
	for (std::set<std::string>::const_iterator set_it = result_set.begin();
				set_it != result_set.end(); ++set_it){
		result.append(*set_it);
	}
	return result;
}


int main(int argc, char *argv[]) {
  assert(argc == 3);

	std::ofstream outfile(argv[2], std::ios::out);

  Vex vex(argv[1]);
  
  Json::Value json_output;
  json_output["exper_name"] = 
    vex.get_root_node()["GLOBAL"]["EXPER"]->to_string();
  json_output["start"] = get_start(vex.get_root_node());
  json_output["stop"] = get_stop(vex.get_root_node());
  for (Vex::Node::const_iterator it = vex.get_root_node()["STATION"]->begin();
       it != vex.get_root_node()["STATION"]->end(); ++it) {
    json_output["stations"].append(it.key());
    json_output["data_sources"][it.key()] = Json::Value(Json::arrayValue);
    json_output["site_position"][it.key()] = site_position(vex.get_root_node(), it.key());
  }

	json_output["channels"] =  get_channels(vex);
  json_output["reference_station"] = "";
  json_output["cross_polarize"]    = false;
  json_output["number_channels"]   = 1024;
  json_output["integr_time"]       = 1;
  json_output["message_level"]     = 0;
  json_output["delay_directory"]   = "";
  json_output["output_file"]       = "";
  json_output["subbands"]          = get_frequencies(vex);

  outfile << json_output << std::endl;
  return 0;
} 
