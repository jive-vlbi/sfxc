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

#include <fstream>

#include "log_writer.h"

class Log_writer_file : public Log_writer {
public:
  Log_writer_file(char *filename, int message_level=0);

};


#endif /*LOG_WRITER_FILE_H*/
