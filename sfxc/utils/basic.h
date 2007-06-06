#ifndef BASIC_H
#define BASIC_H

#include <string>

std::string generate_ccf_filename(const std::string &path,
                                  const std::string &exp,
                                  const std::string &scan_name,
                                  int channel1,
                                  int channel2) {
  char filename[80];
  if (channel2 >= 1) {
    snprintf(filename, 80, "%s/%s_%s_ch%02d_ch%02d.ccf", 
             path.c_str(), exp.c_str(), scan_name.c_str(), channel1, channel2);
  } else {
    snprintf(filename, 80, "%s/%s_%s_ch%d.ccf", 
             path.c_str(), exp.c_str(), scan_name.c_str(), channel1);
  }
  return std::string(filename);
}

std::string generate_dcf_filename(std::string const &path,
                                  std::string const &exp,
                                  const std::string &scan_name,
                                  int channel) {
  char filename[80];
  snprintf(filename, 80, 
           "%s/%s_%s_ch%02d.dcf", 
           path.c_str(), exp.c_str(), scan_name.c_str(), channel);
  return std::string(filename);
}

std::string generate_del_filename(std::string const &path,
                                  std::string const &exp,
                                  const std::string &scan_name,
                                  int channel,
                                  std::string const &station_name) {
  char filename[80];
  snprintf(filename, 80, 
           "%s/%s_%s_ch%02d_%s.del", 
           path.c_str(), exp.c_str(), scan_name.c_str(), 
           channel, station_name.c_str());
  return std::string(filename);
}

std::string generate_cor_filename(const std::string &path,
                                  const std::string &exp,
                                  const std::string &scan_name,
                                  int channel1,
                                  int channel2) {
  char filename[80];
  if (channel2 >= 0) {
    snprintf(filename, 80, "%s/%s_%s_ch%02d_ch%02d.cor", 
             path.c_str(), exp.c_str(), scan_name.c_str(), channel1, channel2);
  } else {
    snprintf(filename, 80, "%s/%s_%s_ch%d.cor", 
             path.c_str(), exp.c_str(), scan_name.c_str(), channel1);
  }
  return std::string(filename);
}


#endif // BASIC_H
