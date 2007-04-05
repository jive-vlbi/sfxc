/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "Log_writer_file.h"
#include <iostream>

Log_writer_file::Log_writer_file(char *filename, int messagelevel, bool interactive) 
  : Log_writer(messagelevel, interactive), out(filename)
{
  
}
  
void Log_writer_file::write_message(const char buff[]) {
  out << buff;
  out.flush();
}
