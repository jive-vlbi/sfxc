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

#include <string>
#include <fstream>
#include <assert.h>

#include "log_writer.h"

class Log_writer_mpi : public Log_writer {
public:
  Log_writer_mpi(int rank, int message_level=0);
};

#endif /*LOG_WRITER_MPI_H_*/
