/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "Log_writer_cout.h"
#include <iostream>

Log_writer_cout::Log_writer_cout(int messagelevel, bool interactive) 
  : Log_writer(messagelevel, interactive)
{
  
}
  
void Log_writer_cout::write_message(const char buff[]) {
  std::cout << buff;
  std::cout.flush();
}
  
void Log_writer_cout::ask_continue() {
  if (!get_interactive()) return;
  char repl; // user reply character
  
  std::cout << "\nEnter c to continue, any other character to stop: ";
  std::cin >> repl;
  if (repl!='c') {
    std::cout << "Application stopped by user!\n";
    exit(0);
  }
} 
