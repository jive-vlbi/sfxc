/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <iostream>

#include "log_writer_void.h"

class Log_writer_void_buffer : public Log_writer_buffer {
public:
  Log_writer_void_buffer(int max_level=0, int buffer_size=160)
    : Log_writer_buffer(max_level, buffer_size) {}
protected:
  int sync() { return 0; }
};

Log_writer_void::Log_writer_void(int messagelevel) 
  : Log_writer(new Log_writer_void_buffer(messagelevel))
{
}
