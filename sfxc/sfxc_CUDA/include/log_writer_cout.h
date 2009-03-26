/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef LOG_WRITER_COUT_H_
#define LOG_WRITER_COUT_H_

#include "log_writer.h"

class Log_writer_cout : public Log_writer {
public:
  Log_writer_cout(int message_level=0);

};

#endif /*LOG_WRITER_COUT_H_*/
