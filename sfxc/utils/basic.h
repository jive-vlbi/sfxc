#ifndef BASIC_H
#define BASIC_H

#include <string>
#include <assert.h>

std::string generate_ccf_filename(const std::string &path,
                                  const std::string &exp,
                                  const std::string &scan_name,
                                  const std::string &channel1,
                                  const std::string &channel2) {
  char filename[80];
  if (channel2.length() >= 1) {
    snprintf(filename, 80, "%s/%s_%s_%s_%s.ccf", 
             path.c_str(), exp.c_str(), scan_name.c_str(), 
             channel1.c_str(), channel2.c_str());
  } else {
    snprintf(filename, 80, "%s/%s_%s_%s.ccf", 
             path.c_str(), exp.c_str(), scan_name.c_str(), channel1.c_str());
  }
  return std::string(filename);
}

std::string generate_dcf_filename(std::string const &path,
                                  std::string const &exp,
                                  const std::string &scan_name,
                                  const std::string &channel) {
  char filename[80];
  snprintf(filename, 80, 
           "%s/%s_%s_%s.dcf", 
           path.c_str(), exp.c_str(), scan_name.c_str(), channel.c_str());
  return std::string(filename);
}

std::string generate_del_filename(std::string const &path,
                                  std::string const &exp,
                                  const std::string &scan_name,
                                  const std::string &channel,
                                  std::string const &station_name) {
  char filename[80];
  snprintf(filename, 80, 
           "%s/%s_%s_%s_%s.del", 
           path.c_str(), exp.c_str(), scan_name.c_str(), 
           channel.c_str(), station_name.c_str());
  return std::string(filename);
}

std::string generate_cor_filename(const std::string &path,
                                  const std::string &exp,
                                  const std::string &scan_name,
                                  std::string const &channel1,
                                  std::string const &channel2) {
  char filename[80];
  if (channel2.size() > 0) {
    snprintf(filename, 80, "%s/%s_%s_%s_%s.cor", 
             path.c_str(), exp.c_str(), scan_name.c_str(), 
             channel1.c_str(), channel2.c_str());
  } else {
    snprintf(filename, 80, "%s/%s_%s_%s.cor", 
             path.c_str(), exp.c_str(), scan_name.c_str(), channel1.c_str());
  }
  return std::string(filename);
}


inline std::vector< std::vector<std::string> >
get_channels(VexPlus const &vex, 
             Json::Value const &sfxc_ctrl) {
  std::vector< std::vector<std::string> > result;
  if (sfxc_ctrl.isMember("channels")) {
    assert(sfxc_ctrl["channels"].isArray());
    for (size_t channel = 0; 
         channel < sfxc_ctrl["channels"].size(); 
         channel++) {
      std::vector<std::string> channel_str;
      assert(sfxc_ctrl["channels"][channel].isArray());
      assert(sfxc_ctrl["channels"][channel].size()<=2);
      for (size_t i = 0; i < sfxc_ctrl["channels"].size(); i++) {
        channel_str.push_back(sfxc_ctrl["channels"][channel][i].asString());
      }
      result.push_back(channel_str);
    }
  } else {
    int nChannels = vex.N_FreqChans(vex.Station(0), vex.Mode(0));
    for (int channel = 0; channel < nChannels; channel ++) {
      std::vector<std::string> channel_str;
      channel_str.push_back(vex.Link_freq_track(vex.Station(0),
                                                vex.Mode(0),
                                                channel));
      
      if ((channel<nChannels-1) &&
          (vex.SkyFreq(vex.Station(0),vex.Mode(0),channel) ==
           vex.SkyFreq(vex.Station(0),vex.Mode(0),channel+1))) {
        channel++;
        channel_str.push_back(vex.Link_freq_track(vex.Station(0),
                                                  vex.Mode(0),
                                                  channel));
      }
      result.push_back(channel_str);
    }
  }
  return result;
}


#endif // BASIC_H
