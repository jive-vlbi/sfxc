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
#include "mark5a_reader.h"

// Prints the timestamps in the headers of a mark5a file
int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  Log_writer_cout log_writer(0);

  if (argc != 2) {
    log_writer << "usage: " << argv[0] << " <mark5a-file>" << std::endl;
    return 1;
  }


  std::tr1::shared_ptr<Data_reader> reader(new Data_reader_file(argv[1]));

  unsigned char buffer[sizeof(int64_t)*SIZE_MK5A_FRAME];

  Mark5a_reader *mark5a_reader =
    get_mark5a_reader(reader, buffer);

  int64_t prev_time = mark5a_reader->get_current_time(), current_time;

  do {
    current_time = mark5a_reader->get_current_time();
    std::cout
    << current_time
    << " \t" << current_time - prev_time
    << std::endl;
    prev_time = current_time;
  } while (mark5a_reader->read_new_block(buffer));

}
