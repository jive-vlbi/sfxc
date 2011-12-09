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

// Prints the timestamps in the headers of a mark5b file
int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " <mark5b-file>" << std::endl;
    return 1;
  }

  FILE *infile = fopen(argv[1], "r");
  if(infile == NULL){
    std::cout << "Could not open " << argv[1] << " for reading.\n";
    return 1;
  }

  find_first_header(infile);
  int header[4];
  int bytes_read = fread(&header[0], 1, MARK5B_HEADER_SIZE, infile);
  while(bytes_read == MARK5B_HEADER_SIZE){
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
      mask = 0x0000000F;
      for(int b = 0, mul = 1; b < 16; b+=4, mul *=10){
        subsec += ((header[3]&mask) >> b) * mul;
        mask = mask << 4;
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
