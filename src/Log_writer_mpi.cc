#include "Log_writer_mpi.h"
#include "sfxc_mpi.h"
#include <assert.h>

Log_writer_mpi::Log_writer_mpi(int rank, int messagelevel, bool interactive) 
  : Log_writer(messagelevel,interactive), rank_str("#---: ")
{
  set_rank(rank);
}
  
void Log_writer_mpi::set_rank(int rank) {
  // Destroys the current message
  assert(rank < 1000);
  rank_str = "#";
  char ch_rank[3];
  itoa(rank, ch_rank, 10);
  rank_str.append(ch_rank);
  rank_str.append(": ");

  msg = rank_str;  
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
  
void Log_writer_mpi::send() {
  MPI_Send((void*)msg.c_str(), msg.size()+1, MPI_CHAR, 
           RANK_LOG_NODE, MPI_TAG_LOG_MESSAGE, MPI_COMM_WORLD);
  msg = rank_str;
}
