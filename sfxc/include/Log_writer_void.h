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
  Log_writer_void(int interactive=0);
};

#endif /*LOG_WRITER_VOID_H*/
