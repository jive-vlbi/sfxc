/* Copyright (c) 2010 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Mark Kettenis <kettenis@jive.nl>, 2010
 *
 */

#ifndef DATA_READER_MK5_H
#define DATA_READER_MK5_H

#include "data_reader.h"

class Data_reader_mk5: public Data_reader {
public:
  Data_reader_mk5(const std::string &);
  ~Data_reader_mk5();

  bool eof();
  bool can_read();

private:
  size_t do_get_bytes(size_t, char *);

  bool at_eof;
  int fd;
};

#endif // DATA_READER_MK5_H
