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
#include "channel_extractor_mark4.h"

#include "utils.h"
#include "mark4_header.h"

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

//  Input_node_tasklet *input_node = get_input_node_tasklet(reader);
  
  

//  Channel_extractor_mark4
//  extractor(reader,
//            false /* insert_random_headers */,
//            Channel_extractor_mark4::CHECK_ALL_HEADERS);
//
//  // print first header
//  extractor.print_header(log_writer,0);
//
//  extractor.goto_next_block();
//
//  // print second header
//  extractor.print_header(log_writer,0);
}
