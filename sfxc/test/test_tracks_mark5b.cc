/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: test_data_reader_mark5.cc 730 2008-04-14 07:51:23Z kruithof $
 *
 * Tests reading data from a Mark5b
*/

#include "utils.h"
#include "control_parameters.h"
#include <fstream>
#include <map>



void get_mark5b_tracks(const Control_parameters &param,
                       const std::string &mode,
                       const std::string &station) {
  const Vex::Node &root=param.get_vex().get_root_node();
  // Find the number of bits per sample
  int bits_per_sample = param.bits_per_sample();
  std::cout << "bits_per_sample: " << bits_per_sample << std::endl;

  std::string bbc = "NO BBC FOUND";
  { // Find the bbc
    Vex::Node::const_iterator bbc_it;
    for (bbc_it = root["MODE"][mode]->begin("BBC");
         bbc_it != root["MODE"][mode]->end("BBC");
         bbc_it ++) {
      Vex::Node::const_iterator station_it = bbc_it->begin(); 
      station_it++;
      while (station_it != bbc_it->end()) {
        if (station_it->to_string() == station)
          bbc = bbc_it->begin()->to_string();
        station_it++;
      }
    }
  }

  std::string freq = "NO FREQ FOUND";
  { // Find the frequency label
    Vex::Node::const_iterator freq_it;
    for (freq_it = root["MODE"][mode]->begin("FREQ");
         freq_it != root["MODE"][mode]->end("FREQ");
         freq_it ++) {
      Vex::Node::const_iterator station_it = freq_it->begin(); 
      station_it++;
      while (station_it != freq_it->end()) {
        if (station_it->to_string() == station)
          freq = freq_it->begin()->to_string();
        station_it++;
      }
    }
  }


  // subband to bit-stream-nr conversion
  std::map<std::string, int> subband_to_track;
  {
    // Sort the bbc's
    std::map<int, std::string> bbc_map;
    Vex::Node::const_iterator bbc_it;
    for (bbc_it = root["BBC"][bbc]->begin("BBC_assign");
         bbc_it != root["BBC"][bbc]->end("BBC_assign");
         bbc_it ++) {
      bbc_map[bbc_it[1]->to_int()] = bbc_it[0]->to_string();
    }
    
    // Sorted list of bbc labels
    std::vector<std::string> bbc_labels;
    bbc_labels.resize(bbc_map.size());
    int i=0;
    for (std::map<int, std::string>::iterator it=bbc_map.begin();
         it != bbc_map.end(); it++) {
      bbc_labels[i] = (*it).second;
      i++;
    }

    { // Iterate over bbcs to find the right numbering of the bit streams
      int bit_stream = 0;
      Vex::Node::const_iterator freq_it;
      // Find the upper sidebands:
      for (size_t bbc_nr=0; bbc_nr < bbc_labels.size(); bbc_nr++) {
        for (freq_it = root["FREQ"][freq]->begin("chan_def");
             freq_it != root["FREQ"][freq]->end("chan_def");
             freq_it++) {
          if ((freq_it[2]->to_string() == std::string("U")) &&
              (freq_it[5]->to_string() == bbc_labels[bbc_nr])){
            subband_to_track[freq_it[4]->to_string()] = bit_stream;
            bit_stream++;
          }
        }
      }
      // Find the lower sidebands:
      for (size_t bbc_nr=0; bbc_nr < bbc_labels.size(); bbc_nr++) {
        for (freq_it = root["FREQ"][freq]->begin("chan_def");
             freq_it != root["FREQ"][freq]->end("chan_def");
             freq_it++) {
          if ((freq_it[2]->to_string() == std::string("L")) &&
              (freq_it[5]->to_string() == bbc_labels[bbc_nr])){
            subband_to_track[freq_it[4]->to_string()] = bit_stream;
            bit_stream++;
          }
        }
      }
    }
  }

  { // Fill the sign and magnitude bits:
    int nr_bit_streams = subband_to_track.size()*bits_per_sample;
    for (size_t ch_nr=0; ch_nr < param.number_frequency_channels(); ch_nr++) {
      const std::string &channel_name = param.frequency_channel(ch_nr);
      int bit_stream_nr = subband_to_track[channel_name]*bits_per_sample;
      std::cout << channel_name << " ";
      if (bits_per_sample == 2) {
        std::cout << "(S,M): ";     
        for (; bit_stream_nr < 32; bit_stream_nr += nr_bit_streams) {
          std::cout << "("<< bit_stream_nr <<"," << (bit_stream_nr+1) << ") ";
        }
        std::cout << std::endl;
      } else {
        std::cout << "S: ";     
        for (; bit_stream_nr < 32; bit_stream_nr += nr_bit_streams) {
          std::cout << bit_stream_nr << " ";
        }
        std::cout << std::endl;
      }
    }
  }
}

void get_all_mark5b_tracks(const Control_parameters &param,
                       const std::string &station) {

  Vex::Node::const_iterator scan_it;
  for (scan_it = param.get_vex().get_root_node()["SCHED"]->begin();
       scan_it != param.get_vex().get_root_node()["SCHED"]->end();
       scan_it++) {
    std::string mode = (*scan_it)["mode"]->to_string();
    std::cout << mode << std::endl;
    get_mark5b_tracks(param, mode, station);
    std::cout << param.get_input_node_parameters(mode, station) << std::endl;
    std::cout << std::endl;
  }
}

int main(int argc, char *argv[]) {
  RANK_OF_NODE = 0;
  
  if (argc != 3) {
    std::cout << "usage: " << argv[0] << " <vex_file> <ctrl_file>"
              << std::endl;
    exit(1);
  }

  char *vex_file = argv[1];
  char *ctrl_file = argv[2];

  Control_parameters param(ctrl_file, vex_file, std::cout);
  Vex::Node::const_iterator station_it;
  for (station_it = param.get_vex().get_root_node()["STATION"]->begin(); 
       station_it != param.get_vex().get_root_node()["STATION"]->end(); 
       station_it++) {
    std::string das = (*station_it)["DAS"]->to_string();

    std::string record_transport_type =
      param.get_vex().get_root_node()["DAS"][das]["record_transport_type"]->to_string();

    if (record_transport_type == "Mark5B") {
      std::cout << station_it.key() << std::endl;
      get_all_mark5b_tracks(param, station_it.key());
    }
  }

  return 0;
}
