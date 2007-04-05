/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef LOG_WRITER_FILE_H
#define LOG_WRITER_FILE_H

#include <Log_writer.h>
#include <fstream>

class Log_writer_file : public Log_writer
{
public:
  Log_writer_file(char *filename, int interactive=0, bool interactive=false);

private:
  void write_message(const char buff[]);
  
  std::ofstream out;
};

#endif /*LOG_WRITER_FILE_H*/
