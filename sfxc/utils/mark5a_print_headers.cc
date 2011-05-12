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
#include "input_node_types.h"
#include "mark5a_reader.h"

typedef Input_node_types::Data_memory_pool Data_memory_pool;

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


  boost::shared_ptr<Data_reader> reader(new Data_reader_file(argv[1]));
  Mark5a_reader *mark5a_reader = new Mark5a_reader(reader, Time(0));

  Mark5a_reader::Data_frame data;
  boost::shared_ptr< Data_memory_pool > memory_pool_(new Data_memory_pool(10));
  data.buffer = memory_pool_->allocate();

  while ((!mark5a_reader->open_input_stream(data)) && (!mark5a_reader->eof()))
    ;
  int64_t prev_time = (int64_t)mark5a_reader->get_current_time().get_time_usec(), current_time;

  do {
    current_time = (int64_t)mark5a_reader->get_current_time().get_time_usec();
    std::cout
    << current_time
    << " \t" << current_time - prev_time
    << std::endl;
    prev_time = current_time;
  } while (mark5a_reader->read_new_block(data));

}
