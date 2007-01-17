#include "Log_writer_void.h"
#include <iostream>

Log_writer_void::Log_writer_void(int messagelevel, bool interactive) 
  : Log_writer(messagelevel)
{
}
  
void Log_writer_void::write_message(const char buff[]) {
}
