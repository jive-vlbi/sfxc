#include "Log_writer_file.h"
#include <iostream>

Log_writer_file::Log_writer_file(char *filename, int messagelevel, bool interactive) 
  : Log_writer(messagelevel, interactive), out(filename)
{
  
}
  
void Log_writer_file::write_message(const char buff[]) {
  out << buff;
  out.flush();
}
