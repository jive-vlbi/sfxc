/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <iostream>

#include "log_writer_cout.h"
#include "data_reader_file.h"

#include "utils.h"
#include "mark4_reader.h"

int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  Log_writer_cout log_writer(0);

  if (argc != 2) {
    log_writer << "usage: " << argv[0] << " <mark4-file>" << std::endl;
    return 1;
  }


  boost::shared_ptr<Data_reader> reader(new Data_reader_file(argv[1]));

  char buffer[sizeof(int64_t)*SIZE_MK4_FRAME];

  Mark4_reader_interface *mark4_reader = get_mark4_reader(reader, buffer);

  int64_t prev_time = mark4_reader->get_current_time(), current_time;

  do {
    current_time = mark4_reader->get_current_time();
    std::cout
    << current_time
    << " \t" << current_time - prev_time
    << std::endl;
    prev_time = current_time;
  } while (mark4_reader->read_new_block(buffer));

}
