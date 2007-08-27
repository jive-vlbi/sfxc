#include <Vex++.h>

#include <assert.h>
#include <string>
#include <stdio.h>

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

std::string get_start(Vex &vex) {
  std::string result = "";
  Vex::iterator sched = vex["SCHED"];
  for (Vex::iterator scan_it = sched->begin();
       scan_it != sched->end(); ++scan_it) {
    if (result == "") {
      result = scan_it["start"]->to_string();
    } else if (result > scan_it["start"]->to_string()) {
      result = scan_it["start"]->to_string();
    }
  }
  return result;
}

std::string get_endtime_of_scan(Vex &scan) {
  std::string start = scan["start"]->to_string();
  int duration = 0;
  for (Vex::iterator it = scan.begin("station");
       it != scan.end("station"); ++it) {
    int curr_duration;
    sscanf(it[2]->to_string().c_str(),"%d sec", &curr_duration);
    duration = (curr_duration > duration ? curr_duration : duration);
  }
  return add_time(start, duration);
}

std::string get_stop(Vex &vex) {
  std::string result = "";
  Vex::iterator sched = vex["SCHED"];
  for (Vex::iterator scan_it = sched->begin();
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

Json::Value get_frequency(Vex &vex,
			  Vex_iterator channel,
			  const std::string &BBC,
			  const std::string &IF) {
  Json::Value result;
  std::string if_def;
  for (Vex::iterator bbc = vex["BBC"][BBC]->begin("BBC_assign");
       bbc != vex["BBC"][BBC]->end("BBC_assign"); ++bbc) {
    if (bbc[0]->to_string() == channel[5]->to_string()) {
      if_def = bbc[2]->to_string();
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
  for (Vex::iterator if_def_it = vex["IF"][IF]->begin("if_def");
       if_def_it != vex["IF"][IF]->end("if_def"); ++if_def_it) {
    if (if_def_it[0]->to_string() == if_def) {
      result["polarisation"] = if_def_it[2]->to_string();
    }
  }
//   result["polarisation"] = vex["IF"][IF];
//   for if_def_it in vex["IF"][IF].getall("if_def"):
//     if if_def_it[0] == if_def:
//       result["polarisation"] = if_def_it[2]
//   return result
  return result;

//   global vex
//   if_def = ""
//   for bbc in vex["BBC"][BBC].getall("BBC_assign"):
//     if bbc[0] == channel[5]:
//       if_def = bbc[2]
  
//   result = dict()
//   result["frequency"]    = float(channel[1].split()[0])*1000000
//   result["bandwidth"]    = float(channel[3].split()[0])*1000000
//   result["sideband"]     = channel[2]+"SB"
//   for if_def_it in vex["IF"][IF].getall("if_def"):
//     if if_def_it[0] == if_def:
//       result["polarisation"] = if_def_it[2]
//   return result

}

Json::Value get_frequencies(Vex &vex) {
  // Find the right BBC and IF
  std::string mode = vex["MODE"]->begin().key();
  std::string IF = vex["MODE"][mode]["IF"][0]->to_string();
  std::string ref_station = vex["MODE"][mode]["IF"][1]->to_string();
  std::string BBC = "";
  // Find the corresponding BBC:
  for (Vex::iterator bbc = vex["MODE"][mode]->begin("BBC");
       bbc != vex["MODE"][mode]->end("BBC"); ++bbc) {
    for (Vex::iterator station = ++(bbc->begin()); 
	 station != (*bbc).end(); ++station) {
      if (ref_station == station->to_string()) {
	BBC = bbc[0]->to_string();
      }
    }
  }

  Json::Value result;
  for (Vex::iterator channel = vex["FREQ"]->begin()->begin("chan_def");
       channel != vex["FREQ"]->begin()->end("chan_def"); ++channel) {
    result.append(get_frequency(vex, channel, BBC, IF));
  }
//   result = []
//   for freq in vex["FREQ"]:
//     for channel in vex["FREQ"][freq].getall("chan_def"):
//       result.append(get_frequency(IF, BBC, channel))
//     break
  return result;
}

Json::Value site_position(Vex &vex,
			  const std::string &station) {
  Json::Value result;

  Vex::iterator position = 
    vex["SITE"][vex["STATION"][station]["SITE"]->to_string()]["site_position"];
  for (Vex::iterator site = position->begin();
       site != position->end(); ++site) {
    double pos;
    int err = sscanf(site->to_string().c_str(), "%lf m", &pos);
    assert(err == 1);
    result.append(pos);
  }
  return result;
}

int main(int argc, char *argv[]) {
  assert(argc == 2);

  struct Vex_node *cvex = parse_vex(argv[1]);
  Vex vex(cvex);
  
  Json::Value json_output;
  json_output["exper_name"] = (*vex["GLOBAL"]["EXPER"]).to_string();
  json_output["start"] = get_start(vex);
  json_output["stop"] = get_stop(vex);
  for (Vex::iterator it = vex["STATION"]->begin();
       it != vex["STATION"]->end(); ++it) {
    json_output["stations"].append(it.key());
    json_output["data_sources"][it.key()] = Json::Value(Json::arrayValue);
    json_output["site_position"][it.key()] = site_position(vex, it.key());
  }
  json_output["reference_station"] = "";
  json_output["cross_polarize"]    = false;
  json_output["number_channels"]   = 1024;
  json_output["integr_time"]       = 1;
  json_output["message_level"]     = 0;
  json_output["delay_directory"]   = "";
  json_output["output_file"]       = "";
  json_output["subbands"]          = get_frequencies(vex);

  std::cout << json_output << std::endl;
  return 0;
} 
