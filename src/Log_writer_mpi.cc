/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "Log_writer_mpi.h"
#include <fstream>
#include <assert.h>
#include <time.h>
#include <sys/timeb.h>
#include <sfxc_mpi.h>

class Log_writer_mpi_buffer : public Log_writer_buffer {
public:
  Log_writer_mpi_buffer(int rank,
                         int messagelevel,
                         int buffer_size=160);
  ~Log_writer_mpi_buffer();

private:
  void	put_buffer(void);
  void	put_char(int);

protected:
  int	overflow(int);
  int	sync();

  int rank;
};

Log_writer_mpi::Log_writer_mpi(int rank,
                               int messagelevel) 
  : Log_writer(new Log_writer_mpi_buffer(rank, messagelevel))
{
}
  
// Buffer
Log_writer_mpi_buffer::Log_writer_mpi_buffer(int rank,
                                             int message_level,
                                             int buffer_size)
 : Log_writer_buffer(message_level, buffer_size),
   rank(rank) {
}

Log_writer_mpi_buffer::~Log_writer_mpi_buffer() {
  sync();
}

int Log_writer_mpi_buffer::overflow(int c) {
  put_buffer();
        
  if (c != EOF)
    if (pbase() == epptr())
      put_char(c);
    else
      sputc(c);
        
  return 0;
}  

int Log_writer_mpi_buffer::sync() {
  put_buffer();
  return 0;
}

void Log_writer_mpi_buffer::put_char(int chr) {
  if (current_level <= max_level)
    std::cout << char(chr);
}
void Log_writer_mpi_buffer::put_buffer() {
  if (pbase() != pptr())
    {
      int     len = (pptr() - pbase());
      assert(len > 0);
      char    *buffer = new char[len + 20 + 1];
        
      if (current_level <= max_level) {
        struct timeb time_struct;
        ftime(&time_struct);
        struct tm *tm_struct = localtime(&time_struct.time);
        snprintf(buffer, 21, "%02dh%02dm%02ds%03dms, %02d, ",
                tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec,
                time_struct.millitm, rank);
        assert(strlen(buffer) == 20);
        strncpy(buffer+20, pbase(), len);
        buffer[len+20] = '\0';

        MPI_Send(buffer, len+20+1, MPI_CHAR, 
                 RANK_LOG_NODE, MPI_TAG_LOG_MESSAGE, MPI_COMM_WORLD);
      }
        
      setp(pbase(), epptr());
      delete [] buffer;
    }
}
