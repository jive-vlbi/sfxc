#include "Log_writer_cout.h"
#include <iostream>

Log_writer_cout::Log_writer_cout(int messagelevel, bool interactive) 
  : Log_writer(messagelevel), _interactive(interactive)
{
  
}
  
void Log_writer_cout::write_message(const char buff[]) {
  std::cout << buff;
}
  
void Log_writer_cout::ask_continue() {
  if (!_interactive) return;
  char repl; // user reply character
  
  std::cout << "\nEnter c to continue, any other character to stop: ";
  std::cin >> repl;
  std::cout << std::endl;
  if (strcmp(&repl,"c")!=0) {
    std::cout << "Application stopped by user!\n";
    exit(0);
  }
} 
