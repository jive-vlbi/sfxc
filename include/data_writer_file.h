/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef DATA_WRITER_FILE_H
#define DATA_WRITER_FILE_H

#include <fstream>
#include <vector>

#include "data_writer.h"

class Data_writer_file : public Data_writer {
public:
  Data_writer_file(const char *filename);
  ~Data_writer_file();
  
  size_t do_put_bytes(size_t nBytes, const char *buff);

private:
  std::ofstream file;
};

#endif // DATA_WRITER_FILE_H
