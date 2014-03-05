/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Aard Keimpema <keimpema@JIVE.nl>, 2011
 *
 */

#include <iostream>
#include <string>
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

int compute_mjd(int day, int month, int year)
// Calculate the modified julian day, formula taken from the all knowing wikipedia
{
  int a = (14-month)/12;
  int y = year + 4800 - a;
  int m = month + 12*a - 3;
  int jdn = day + ((153*m+2)/5) + 365*y + (y/4) - (y/100) + (y/400) - 32045;
  return jdn - 2400000.5;
}

void get_date(int mjd, int *year, int *day){
// Valid from 1-1-1901 to 1-1-2100
  int mjd1901 = 15385;
  int ndays = mjd - mjd1901;
  int p1 = ndays / (365 * 4 + 1);
  int p2 = (ndays - p1 * (365 * 4 + 1)) / 365;
  *year = 4 * p1 + p2 + 1901;
  *day = 1 + (ndays - p1 * (365 * 4 + 1)) % 365;
}

std::string get_time_string(int mjd, int sec){
  char tstr[80];
  int year, yday, hour, minute;

  get_date(mjd, &year, &yday);
  hour = sec / 3600;
  minute = (sec%3600)/60;

  snprintf(tstr, 80, "%dy%03dd%02dh%02dm%02ds", year, yday, hour, minute, sec%60);
  return(std::string(tstr));
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
      // Compute / guess the full MJD
      int full_mjd;
      if (year > 0){
        int mjd_year = compute_mjd(1, 1, year);
        if (mjd >= mjd_year%1000)
          full_mjd = mjd_year + mjd - mjd_year%1000;
        else
          full_mjd = mjd_year + mjd - mjd_year%1000 + 1000;
      }else{
        time_t rawtime;
        struct tm *now;
        time(&rawtime);
        now = gmtime(&rawtime);
        int today_mjd = compute_mjd(now->tm_mday,now->tm_mon+1,now->tm_year+1900);
        full_mjd = (today_mjd/1000)*1000 + mjd;
        if (full_mjd > today_mjd)
          full_mjd -= 1000;
      }
      std::string time_string = get_time_string(full_mjd, sec);
      std::cout << "mjd = " << mjd << ", sec = " << sec << ", subsec = " << subsec 
                << ", frame_nr = " << frame_nr << "; t = " << time_string << "\n";
    }
    fseek(infile, MARK5B_FRAME_SIZE, SEEK_CUR);
    bytes_read = fread(&header[0], 1, MARK5B_HEADER_SIZE, infile);
  }
  return 0;
}
