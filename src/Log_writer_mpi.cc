/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "Log_writer_mpi.h"
#include "sfxc_mpi.h"
#include <assert.h>
#include <time.h>
#include <sys/timeb.h>

Log_writer_mpi::Log_writer_mpi(int rank, int messagelevel, bool interactive) 
  : Log_writer(messagelevel,interactive)
{
  set_rank(rank);
  set_prefix();
}
  
void Log_writer_mpi::set_rank(int rank_) {
  rank = rank_;
}

void Log_writer_mpi::write_message(const char buff[]) {
  char *end=strchr(buff, '\n');
  while (end != NULL) {
    assert(*end == '\n');
    // Don't send the '\n', a newline is always given in the Log_controller
    msg.append(buff, end-buff);
    send();
    buff = end+1;
    end=strchr(buff, '\n');
  }
    
  msg.append(buff);
}

void Log_writer_mpi::set_prefix() {
  char prefix[80];
  struct timeb time_struct;
  ftime(&time_struct);
  struct tm *tm_struct = localtime(&time_struct.time);
  sprintf(prefix, "%02dh%02dm%02ds%03dms, %d, ", 
          tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec,
          time_struct.millitm, rank);
  msg = prefix;  
}
  
void Log_writer_mpi::send() {
  MPI_Send((void*)msg.c_str(), msg.size()+1, MPI_CHAR, 
           RANK_LOG_NODE, MPI_TAG_LOG_MESSAGE, MPI_COMM_WORLD);

  set_prefix();
}
