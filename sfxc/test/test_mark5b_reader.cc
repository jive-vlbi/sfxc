/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: test_data_reader_mark5.cc 730 2008-04-14 07:51:23Z kruithof $
 *
 * Tests reading data from a Mark5b
*/

#include "mark5b_reader.h"
#include "data_reader_file.h"

int main(int argc, char *argv[]) {
  RANK_OF_NODE = 0;
  
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " <mk5b_file>" << std::endl;
    exit(1);
  }

  char *filename = argv[1];
  boost::shared_ptr<Data_reader> reader(new Data_reader_file(filename));
  Mark5b_reader mk5b_reader(reader);

  unsigned char data[16*2500*sizeof(int32_t)];

  while (!mk5b_reader.eof()) {
    mk5b_reader.read_new_block((unsigned char *)&data[0]);
    std::cout << mk5b_reader.get_current_time() << std::endl;
  }

//   // Check goto time:
//   std::cout << mk5b_reader.goto_time(data, mk5b_reader.get_current_time())
//             << std::endl;
//   std::cout << mk5b_reader.goto_time(data, mk5b_reader.get_current_time()+300)
//             << std::endl;
//   std::cout << mk5b_reader.goto_time(data, mk5b_reader.get_current_time()+600)
//             << std::endl;

  return 0;
}
