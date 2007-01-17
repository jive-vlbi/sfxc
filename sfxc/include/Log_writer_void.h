#ifndef LOG_WRITER_VOID_H
#define LOG_WRITER_VOID_H

#include <Log_writer.h>

class Log_writer_void : public Log_writer
{
public:
  Log_writer_void(int messagelevel, bool interactive=false);
  
  void set_interactive(int i) { }
  int  get_interactive()      { return false; }
private:
  void write_message(const char buff[]);
};

#endif /*LOG_WRITER_VOID_H*/
