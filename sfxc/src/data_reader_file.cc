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
#include <string.h>
#include <iostream>
#include <algorithm>

Data_reader_file::Data_reader_file(const char *filename) :
    Data_reader() {
  if (strncmp(filename, "file://", 7) != 0) {
    std::string msg = std::string("Filename '")+std::string(filename)+std::string("' doesn't start with file://");
    sfxc_abort(msg.c_str());
  }
  file.open(filename+7, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::string msg = std::string("Filename '")+std::string(filename)+std::string("' doesn't exist");
    sfxc_abort(msg.c_str());
  }
  is_seekable_ = true;
}

Data_reader_file::Data_reader_file(const std::string &filename) :
    Data_reader() {
  if (strncmp(filename.c_str(), "file://", 7) != 0) {
    std::string msg = std::string("Filename '")+filename+std::string("' doesn't start with file://");
    sfxc_abort(msg.c_str());
  }
  file.open(filename.c_str()+7, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    std::string msg = std::string("Filename '")+filename+std::string("' doesn't exist");
    sfxc_abort(msg.c_str());
  }
  is_seekable_ = true;
}

Data_reader_file::~Data_reader_file() {
  file.close();
}

size_t
Data_reader_file::do_get_bytes(size_t nBytes, char*out) {
  if (!file.good()) {
    return -1;
  }
  if (out == NULL) {
    uint64_t pos = file.tellg();
    file.seekg (nBytes, std::ios::cur);
    uint64_t pos2 = file.tellg();
    return pos2 - pos;
  }
  file.read(out, nBytes);
  if (file.eof()) return file.gcount();
  return nBytes;
}

bool Data_reader_file::eof() {
  return file.eof() || !file.good();
}

bool Data_reader_file::can_read() {
  DEBUG_MSG("Data_reader_file: can read not implemented");
  return true;
}
