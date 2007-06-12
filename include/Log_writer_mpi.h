/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef LOG_WRITER_MPI_H_
#define LOG_WRITER_MPI_H_

#include "Log_writer.h"
#include <string>

class Log_writer_mpi : public Log_writer
{
public:
  Log_writer_mpi(int rank, int messagelevel=0, bool interactive=false);
  
  void set_rank(int rank);
private:
  void send();
  void set_prefix();
  void write_message(const char buff[]);
  std::string msg;
  int rank;
};


#endif /*LOG_WRITER_MPI_H_*/
