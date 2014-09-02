/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 */

#ifndef DATA_READER_FILE_H
#define DATA_READER_FILE_H

#include <queue>
#include <vector>
#include <fstream>

#include "data_reader.h"

class Data_reader_file : public Data_reader {
public:
  Data_reader_file(const std::vector<std::string> &sources);
  Data_reader_file(const std::string &source);
  ~Data_reader_file();

  bool eof();
  bool can_read();

private:
  void init(const std::vector<std::string> &sources);
  bool open_next_file();
  size_t do_get_bytes(size_t nBytes, char *out);

  std::queue<std::string> filenames;
  std::ifstream file;
};

#endif // DATA_READER_FILE_H
