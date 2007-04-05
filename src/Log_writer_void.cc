/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "Log_writer_void.h"
#include <iostream>

Log_writer_void::Log_writer_void(int messagelevel, bool interactive) 
  : Log_writer(messagelevel)
{
}
  
void Log_writer_void::write_message(const char buff[]) {
}
