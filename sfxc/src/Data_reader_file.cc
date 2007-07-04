/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Data_reader_file.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include <utils.h>

Data_reader_file::Data_reader_file(char *filename) : 
  Data_reader()
{
  file.open(filename, std::ios::in | std::ios::binary);
  assert(file.is_open() );
  assert(file.good());
}

Data_reader_file::~Data_reader_file() {
  file.close();
}

size_t
Data_reader_file::do_get_bytes(size_t nBytes, char*out) {
  assert(file.good());
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
  return file.eof(); 
}
