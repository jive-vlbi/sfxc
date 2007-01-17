#ifndef LOG_WRITER_COUT_H_
#define LOG_WRITER_COUT_H_

#include <Log_writer.h>

class Log_writer_cout : public Log_writer
{
public:
  Log_writer_cout(int interactive, bool interactive);
  
  void ask_continue();

  void set_interactive(int i) { _interactive = i; }
  int  get_interactive()      { return _interactive; }


private:
  void write_message(const char buff[]);
  int _interactive;
};

#endif /*LOG_WRITER_COUT_H_*/
