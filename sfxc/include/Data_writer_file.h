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

#include <Data_writer.h>

#include <vector>

class Data_writer_file : public Data_writer {
public:
  Data_writer_file(const char *filename);
  ~Data_writer_file();
  
  INT64 do_put_bytes(INT64 nBytes, char *buff);

private:
  FILE *file;
};

#endif // DATA_WRITER_FILE_H
