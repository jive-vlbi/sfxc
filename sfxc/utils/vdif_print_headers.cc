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
#include <getopt.h>
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

void get_options(int argc, char*argv[], char **filename, int &size_frame){
  int c, opt_index = 0;
  static struct option long_options[] = {
      {"help",    no_argument, 0, 'h'},
      {"size_frame",    required_argument, 0, 's'},
      {0, 0, 0, 0}
  };
  size_frame = 0;
  while( (c = getopt_long (argc, argv, "hs:",  long_options, 
                           &opt_index)) != -1){
    switch (c){
    case 'h':
      std::cout << "usage: " << filename << "[-s frame_size] <vdif-file>\n";
      exit(0);
      break;
    case 's':
      sscanf(optarg, "%d", &size_frame);
      break;
    default:
      printf ("option %s", long_options[opt_index].name);
      abort();
    }
  }
  if ((argc-optind) != 1) {
    std::cout << "usage: " << filename << "[-s frame_size] <vdif-file>\n";
    exit(1);
  }
  *filename = argv[optind];
}
 
int main(int argc, char *argv[]) {
  int size_frame;
  char *filename;
  get_options(argc, argv, &filename, size_frame);

  FILE *infile = fopen(filename, "r");
  if(infile == NULL){
    std::cout << "Could not open " << filename << " for reading.\n";
    return 1;
  }

  bool eof = false;
  int invalid_nr = 0;
  size_t data_size = 1, header_size = 0;
  while (eof == false){
    Header header;
    unsigned char *header_buf = (unsigned char *) &header;
    size_t nwords = fread(&header_buf[0],4,4,infile);
    if (header.legacy_mode == 0)
      nwords = fread(&header_buf[16],4,4,infile);
    if (nwords == 4){
      if (((uint32_t *)header_buf)[0] == 0x11223344 ||
          ((uint32_t *)header_buf)[1] == 0x11223344 ||
          ((uint32_t *)header_buf)[2] == 0x11223344 ||
          ((uint32_t *)header_buf)[3] == 0x11223344){
        std::cout << "Invalid frame with fill pattern : Nr of previous bad frames = " << invalid_nr << "\n";
        invalid_nr++;
      }else{
        Time t = get_time(header.ref_epoch, header.sec_from_epoch);
        data_size = 8*header.dataframe_length;
        header_size = (16+16*(1-header.legacy_mode));
        uint16_t s_id = header.station_id;
        char *s = (char *)&s_id;
        char station[3];
        if (isalnum(s[0]) && isalnum(s[1])){
          station[0] = s[1];
          station[1] = s[0];
          station[2] = 0;
        }else{
          station[0] = ' ';
          station[1] = ' ';
          station[2] = 0;
        }
        invalid_nr = 0;
        std::cout << t << " ,frame_nr = " << header.dataframe_in_second 
                  << ", thread_id = " << header.thread_id << ", nchan = " << (1 << header.log2_nchan)
                  << ", invalid = " << (int)header.invalid << ", legacy = " << (int)header.legacy_mode
                  << ", station = " << station
                  << ", bps-1 = " << (int) header.bits_per_sample
                  << ", data_size = " << data_size << "\n";
      }
      if (size_frame > 0){
        // The frame size in the header is overridden
        fseek(infile, size_frame-header_size, SEEK_CUR);
      }else{
        fseek(infile, data_size-header_size, SEEK_CUR);
      }
    } else {
      eof = true;
    }
  }
  return 0;
}
