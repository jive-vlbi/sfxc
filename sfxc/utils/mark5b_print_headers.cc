/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Aard Keimpema <keimpema@JIVE.nl>, 2011
 *
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define MARK5B_HEADER_SIZE 16
#define MARK5B_FRAME_SIZE 10000
#define SYNCWORD 0xabaddeed

void find_first_header(FILE *infile){
 int word=0;
 int bytes_read = fread(&word, 1, 4, infile);
 while (word != SYNCWORD){
   bytes_read = fread(&word, 1, 4, infile);
   if (bytes_read != 4){
     std::cout << "Could not find mark5b syncword before end of file\n";
     exit(1);
   }
 }
 fseek(infile, -4, SEEK_CUR);
}

void usage(char *filename){
  std::cout << "Usage : " << filename << " [OPTIONS] <filename>\n"
            << "Options : -n <number>, Only print <number> timestamps\n"
            << "          -y <YEAR>, the year in which the recording was made\n";
}

void parse_arguments(int argc, char *argv[], char **filename, int *n_time_stamps, int *year){
  int c;
  *n_time_stamps = -1;
  *year = 0;
    
  while ((c = getopt (argc, argv, "n:y:")) != -1){
    bool error = false;
    char *next;

    switch (c){
    case 'n':
      next = optarg;
      *n_time_stamps = strtol(optarg, &next, 10);
      error = (next == optarg);
     break;
    case 'y':
      next = optarg;
      *year = strtol(optarg, &next, 10);
      error = (next == optarg);
      break;
    case '?':
      std::cerr << "Error : Invalid option, " << (char)optopt << "\n";
      usage(argv[0]);
      exit(1);
    }
    if (error){
      std::cerr << "Error : invalid parameter\n";
      exit(1);
    }
  }
  if(argc - optind != 1){
    std::cerr << "Invalid number of arguments\n";
    usage(argv[0]);
    exit(1);
  }
  *filename = argv[optind];
}


// Prints the timestamps in the headers of a mark5b file
int main(int argc, char *argv[]) {
  int n_time_stamps=-1, year=0;
  char *filename; 

  parse_arguments(argc, argv, &filename, &n_time_stamps, &year);

  FILE *infile = fopen(filename, "r");
  if(infile == NULL){
    std::cout << "Could not open " << filename << " for reading.\n";
    return 1;
  }

  find_first_header(infile);
  int header[4];
  int bytes_read = fread(&header[0], 1, MARK5B_HEADER_SIZE, infile);
  int n=0;
  while(bytes_read == MARK5B_HEADER_SIZE){
    if (n++ == n_time_stamps)
      break; 
    if (header[0] != SYNCWORD){
      std::cout << "Invalid header\n";
    }else{
      int frame_nr = header[1]&0x7FFF;
      int sec = 0, subsec = 0, mjd = 0;
      unsigned int mask = 0x00F00000;
      for(int b = 20, mul = 1; b < 32; b+=4, mul *=10){
        mjd += ((header[2]&mask) >> b) * mul;
        mask = mask << 4;
      }
      mask = 0x0000000F;
      for(int b = 0, mul = 1; b < 20; b+=4, mul *=10){
        sec += ((header[2]&mask) >> b) * mul;
        mask = mask << 4;
      }
      mask = 0x000F0000;
      for(int b = 16, mul = 1; b < 32; b+=4, mul *=10){
        subsec += ((header[3]&mask) >> b) * mul;
        mask = mask << 4;
      }
      // If a year is given then compute the full MJD
      if (year > 0){
        int y = year + 4799;
        int mjd_year = 365*y + (y/4) - (y/100) + (y/400) - 2431738.5; 
        if (mjd >= mjd_year%1000)
          mjd = mjd_year + mjd - mjd_year%1000;
        else
          mjd = mjd_year + mjd - mjd_year%1000 + 1000;
      }
      char time_string[80];
      sprintf(time_string, "%02d:%02d:%02d.%d", sec/(60*60), (sec%(60*60))/60, sec%60, subsec);
      std::cout << "mjd = " << mjd << ", sec = " << sec << ", subsec = " << subsec 
                << ", frame_nr = " << frame_nr << "; t = " << time_string << "\n";
    }
    fseek(infile, MARK5B_FRAME_SIZE, SEEK_CUR);
    bytes_read = fread(&header[0], 1, MARK5B_HEADER_SIZE, infile);
  }
  return 0;
}
