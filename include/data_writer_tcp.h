/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef DATA_WRITER_TCP_H
#define DATA_WRITER_TCP_H

#include <vector>

#include "data_writer.h"
#include "tcp_connection.h"

class Data_writer_tcp : public Data_writer {
public:
  Data_writer_tcp();
  void open_connection(TCP_Connection &tcp_connection);

  ~Data_writer_tcp();
  
  void flush();
private:
  size_t do_put_bytes(size_t nBytes, const char *buff);

  int socket;
};

#endif // DATA_WRITER_TCP_H
