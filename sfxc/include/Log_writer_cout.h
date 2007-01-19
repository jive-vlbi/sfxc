#ifndef LOG_WRITER_COUT_H_
#define LOG_WRITER_COUT_H_

#include <Log_writer.h>

class Log_writer_cout : public Log_writer
{
public:
  Log_writer_cout(int interactive=0, bool interactive=false);
  
  void ask_continue();
private:
  void write_message(const char buff[]);
};

#endif /*LOG_WRITER_COUT_H_*/
