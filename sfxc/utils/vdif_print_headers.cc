/* Copyright (c) 2012 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Aard Keimpema <keimpema@JIVE.nl>, 2012
 *
 */
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "correlator_time.h"

struct Header {
  // Word 0
  uint32_t      sec_from_epoch:30;
  uint8_t       legacy_mode:1, invalid:1;
  // Word 1
  uint32_t      dataframe_in_second:24;
  uint8_t       ref_epoch:6, unassiged:2;
  // Word 2
  uint32_t      dataframe_length:24;
  uint8_t       log2_nchan:5, version:3;
  // Word 3
  uint16_t      station_id:16, thread_id:10;
  uint8_t       bits_per_sample:5, data_type:1;
  // Word 4
  uint32_t      user_data1:24;
  uint8_t       edv:8;
  // Word 5-7
  uint32_t      user_data2,user_data3,user_data4;
};

Time get_time(int ref_epoch, int sec_from_epoch){
  Time time;
  int ref_year = 2000+ref_epoch/2;
  int ref_month = 1+6*(ref_epoch&1);
  int ref_mjd = mjd(1,ref_month,ref_year);
  time.set_time(ref_mjd, sec_from_epoch);
  return time;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " <vdif-file>" << std::endl;
    return 1;
  }

  FILE *infile = fopen(argv[1], "r");
  if(infile == NULL){
    std::cout << "Could not open " << argv[1] << " for reading.\n";
    return 1;
  }

  bool eof = false;
  while (eof == false){
    Header header;
    unsigned char *header_buf = (unsigned char *) &header;
    size_t nwords = fread(&header_buf[0],4,4,infile);
    if (header.legacy_mode == 0)
      nwords = fread(&header_buf[16],4,4,infile);
    if (nwords == 4){
      Time t = get_time(header.ref_epoch, header.sec_from_epoch);
      size_t data_size = 8*header.dataframe_length - (16+16*(1-header.legacy_mode));
      uint16_t s_id = header.station_id;
      char *s = (char *)&s_id;
      char station[3];
      station[0] = s[1];
      station[1] = s[0];
      station[2] = 0;
      std::cout << t << " ,frame_nr = " << header.dataframe_in_second 
                << ", thread = " << header.thread_id << " / " << (1 << header.log2_nchan)-1
                << ", invalid = " << (int)header.invalid << ", station = " << station
                << ", bps = " << header.bits_per_sample+1 
                << ", data_size = " << data_size << "\n";
      fseek(infile, data_size, SEEK_CUR);
    } else {
      eof = true;
    }
  }
  return 0;
}
