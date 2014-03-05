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
#include <string.h>
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

void usage(char *filename){
  std::cout << "usage: " << filename << "[OPTIONS] <vdif-file>\n";
  std::cout << "Options :\n";
  std::cout << "  -q, --quiet          Only print bad frames\n"; 
  std::cout << "  -s, --skip=N         Skip the first N frames.\n"; 
  std::cout << "  -n, --n-frames=M     Only print the first M frames.\n"; 
  std::cout << "  -f, --frame-size=P   Override the frame size given in the VDIF headers..\n";
}

void get_options(int argc, char*argv[], char **filename, int &frame_size, int &nskip, int &nframes, bool &quiet){
  int c, opt_index = 0;
  static struct option long_options[] = {
      {"help",    no_argument, 0, 'h'},
      {"frame-size",    required_argument, 0, 'f'},
      {"skip",    required_argument, 0, 's'},
      {"nframes",  required_argument, 0, 'n'},
      {"quiet",    required_argument, 0, 'q'},
      {0, 0, 0, 0}
  };
  frame_size = 0;
  nskip = 0;
  quiet = false;
  nframes = -1;
  bool error = false;
  while( (c = getopt_long (argc, argv, "qhs:f:n:",  long_options, 
                           &opt_index)) != -1){
    int i;
    switch (c){
    case 'h':
      usage(argv[0]);
      exit(0);
      break;
    case 'q':
      quiet = true;
      break;
    case 'n':
      i = sscanf(optarg, "%d", &nframes);
      error = (i == 0);
      break;
    case 's':
      i = sscanf(optarg, "%d", &nskip);
      error = (i == 0);
      break;
    case 'f':
      i = sscanf(optarg, "%d", &frame_size);
      error = (i == 0);
      break;
    default:
      printf ("option %s", long_options[opt_index].name);
      abort();
    }
    if (error){
      printf("Bad argument to option %s\n", long_options[opt_index].name);
      abort();
    }
  }
  if ((argc-optind) != 1) {
    std::cout << "Input file not specified\n";
    usage(argv[0]);
    exit(1);
  }
  if ((nskip > 0) && (frame_size <= 0)){
    std::cout << "Skip frames requires frame size to be give.\n";
    exit(1);
  }
  *filename = argv[optind];
}

void print_header(Header &header){
  Time t = get_time(header.ref_epoch, header.sec_from_epoch);
  int data_size = 8*header.dataframe_length;
  int header_size = (16+16*(1-header.legacy_mode));
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
  std::cout << t << " ,frame_nr = " << header.dataframe_in_second 
            << ", thread_id = " << header.thread_id << ", nchan = " << (1 << header.log2_nchan)
            << ", invalid = " << (int)header.invalid << ", legacy = " << (int)header.legacy_mode
            << ", station = " << station
            << ", bps-1 = " << (int) header.bits_per_sample
            << ", data_size = " << data_size << "\n";
}

bool check_header(Header &header, Header &first_header){
  // Check if current header is valid by comparing again a previous valid header
  // FIXME this can be done more efficient
  if(header.legacy_mode != first_header.legacy_mode)
    return false; 
  if(header.ref_epoch != first_header.ref_epoch)
    return false; 
  if(header.dataframe_length != first_header.dataframe_length)
    return false; 
  if(header.log2_nchan != first_header.log2_nchan)
    return false; 
  if(header.version != first_header.version)
    return false; 
  if(header.station_id != first_header.station_id)
    return false; 
  if(header.bits_per_sample != first_header.bits_per_sample)
    return false; 
  if(header.data_type != first_header.data_type)
    return false; 
  return true;
}

int64_t find_next_valid_header(FILE *infile, Header &prev_header){
  // Move file pointer to where the current header was supposed to start
  fseek(infile, -sizeof(Header), SEEK_CUR);

  const int N = 1024;
  const int hsize = sizeof(Header);
  unsigned char buf[N];
  size_t nBytes = fread(buf, 1, N, infile);
  size_t total_bytes=0;
  while (nBytes > 0){
    int i;
    for(i=0;i<N-hsize;i++){
      Header *new_header = (Header *)&buf[i];
      if (check_header(*new_header, prev_header)){
        // Found new valid header
        fseek(infile, -N+i, SEEK_CUR);
        return total_bytes + i; 
      }
    }
    memcpy(&buf[0], &buf[i], hsize);
    nBytes = fread(&buf[hsize], 1, N-hsize, infile);
    total_bytes += nBytes;
  }
  // EOF is reached
  return 0;
}

int main(int argc, char *argv[]) {
  int frame_size, nskip, nframes=-1;
  bool quiet;
  char *filename;
  get_options(argc, argv, &filename, frame_size, nskip, nframes, quiet);

  FILE *infile = fopen(filename, "r");
  if(infile == NULL){
    std::cout << "Could not open " << filename << " for reading.\n";
    return 1;
  }
  // Skip the first nskip frames
  fseek(infile, frame_size * nskip, SEEK_SET);

  bool eof = false;
  int invalid_nr = 0;
  int n = 0;
  size_t data_size = 1, header_size = 0;
  bool first_header = true;
  Header prev_header;
  while ((n++ != nframes) && (eof == false)){
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
        // We save the first header to be able to determine if a frame is valid or not
        if (first_header){
          first_header = false;
          memcpy(&prev_header, header_buf, sizeof(Header));
        }
        if (check_header(header, prev_header)){
          data_size = 8*header.dataframe_length;
          header_size = (16+16*(1-header.legacy_mode));
          invalid_nr = 0;
          // Save the last valid header
          memcpy(&prev_header, header_buf, sizeof(Header));
          if(!quiet)
            print_header(header);
        }else{
          int64_t nBytes = find_next_valid_header(infile, prev_header);
          if (nBytes <=0){
            std::cout << "INVALID HEADER found, no new valid header before EOF.\n";
            if(quiet){
              std::cout << "Last valid header : \n";
              print_header(header);
            }
            exit(1);
          }else{
            std::cout << "INVALID HEADER found, found new header after " << nBytes << " bytes = ";
            if (frame_size > 0)
              std::cout << nBytes * 1. / frame_size << " frames\n";
            else
              std::cout << nBytes * 1. /data_size << " frames\n";
            if(quiet){
              std::cout << "Previous valid header : \n";
              print_header(prev_header);
            }
            continue;
          }
        }
      }
      if (frame_size > 0){
        // The frame size in the header is overridden
        fseek(infile, frame_size-header_size, SEEK_CUR);
      }else{
        fseek(infile, data_size-header_size, SEEK_CUR);
      }
    } else {
      eof = true;
    }
  }
  return 0;
}
