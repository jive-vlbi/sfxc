#ifndef LOG_WRITER_MPI_H_
#define LOG_WRITER_MPI_H_

#include "Log_writer.h"
#include <string>

class Log_writer_mpi : public Log_writer
{
public:
  Log_writer_mpi(int interactive=0, bool interactive=false);
  
  void set_rank(int rank);
private:
  void send();
  void write_message(const char buff[]);
  std::string rank_str, msg;
};


#endif /*LOG_WRITER_MPI_H_*/
