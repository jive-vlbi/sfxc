/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef LOG_WRITER_VOID_H
#define LOG_WRITER_VOID_H

#include <Log_writer.h>

class Log_writer_void : public Log_writer
{
public:
  Log_writer_void(int messagelevel=0, bool interactive=false);
  
  void set_interactive(int i) { }
  int  get_interactive()      { return false; }
private:
  void write_message(const char buff[]);
};

#endif /*LOG_WRITER_VOID_H*/
