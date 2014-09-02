/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "data_reader_file.h"
#include "utils.h"
#include <string>
#include <iostream>

Data_reader_file::Data_reader_file(const std::vector<std::string> &sources) :
  Data_reader() {
  init(sources);
}

Data_reader_file::Data_reader_file(const std::string &source) :
  Data_reader() {
  std::vector<std::string> sources(1, source);
  init(sources);
}

void
Data_reader_file::init(const std::vector<std::string> &sources)
{
  for (int i = 0; i < sources.size(); i++) {
    SFXC_ASSERT(sources[i].compare(0, 7, "file://") == 0);
    filenames.push(sources[i].substr(7));
  }
  if(!open_next_file()){
    sfxc_abort("Could not open any input files");
  }
  is_seekable_ = true;
}

bool
Data_reader_file::open_next_file(){
  bool opened_file = false;
  if (file.is_open())
    file.close();

  while ((!opened_file) && (filenames.size() > 0)) {
    file.open(filenames.front().c_str(), std::ios::in | std::ios::binary);
    if (file.is_open()) 
      opened_file = true;
    else
      std::cerr << RANK_OF_NODE << " : Warning : Cannot open " <<  filenames.front() << "\n";
    filenames.pop();
  }
  return opened_file;
}

Data_reader_file::~Data_reader_file() {
  file.close();
}

size_t
Data_reader_file::do_get_bytes(size_t nbytes, char *out) {
  if (!file.good()){
    if (!open_next_file())
      return -1;
  }

  if (out == NULL) {
    std::streampos pos = file.tellg();
    file.seekg(nbytes, std::ios::cur);
    return file.tellg() - pos;
  }

  file.read(out, nbytes);

  if (file.eof()) {
    nbytes = file.gcount();
    open_next_file();
  }

  return nbytes;
}

bool Data_reader_file::eof() {
  return file.eof() || !file.good();
}

bool Data_reader_file::can_read() {
  DEBUG_MSG("Data_reader_file: can read not implemented");
  return true;
}
