/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: plot_headers.cc 278 2007-07-04 07:27:05Z kruithof $
 *
 */

#include <iostream>

#include <Log_writer_cout.h>
#include <Data_reader_file.h>
#include <Channel_extractor_mark4.h>

#include <genFunctions.h>
#include <constPrms.h>

#include <utils.h>
#include <Mark4_header.h>

int main(int argc, char *argv[]) {
  Log_writer_cout log_writer(0);
  
  if (argc != 2) {
    log_writer << "usage: " << argv[0] << " <mark4-file>" << std::endl;
    log_writer << "Plots the first mark4 header" << std::endl;
    return 1;
  }

  Data_reader_file data_reader(argv[1]);
  int nBytes = frameMk4*sizeof(int64_t);
  int32_t data_frame[frameMk4];
  
  data_reader.get_bytes(nBytes, (char*)data_frame);
  
  Mark4_header<int32_t> header;
  header.set_header(data_frame);
  header.check_header();
  
  for (int i=0; i<1; i++) {
    std::cout << header.year(i) << "y"
              << header.day(i) << "d"
              << header.hour(i) << "h"
              << header.minute(i) << "m"
              << header.second(i) << "s"
              << header.microsecond(i) << "ms"
              << std::endl;
  }
  std::cout << std::endl;

}
