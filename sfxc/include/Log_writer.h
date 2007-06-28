/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef LOG_WRITER_H_
#define LOG_WRITER_H_

#include <string>
#include <sstream>
#include <iostream>
#include "types.h"

using namespace std;

/// Conversion of an integer-type to a character.
template <class T>
char* itoa(T value, char* result, int base ) {
  // check that the base if valid
  if (base < 2 || base > 16) { *result = 0; return result; }
  
  char* out = result;
  T quotient = value;

  do {
    *out = "0123456789abcdef"[ (quotient<T(0) ? -1 : 1)*(quotient % base) ];
    ++out;
    quotient /= base;
  } while ( quotient );
  
  // Only apply negative sign for base 10
  if ( (value < T(0)) && (base == 10) ) *out++ = '-';
  
  std::reverse( result, out );
  *out = 0;
  
  return result;
}

class Log_writer {
public:
  Log_writer(int messagelevel=0, bool interactive=false);
  
  virtual ~Log_writer();
  
  /** Writes a message if messagelevel is greater than the set level
  **/ 
  void message(int messagelevel, const char buff[]);
  /** Writes a message if messagelevel is greater than the set level
  **/ 
  void message(int messagelevel, std::string const &msg);
  /** Writes a message if messagelevel is greater than the set level
  **/ 
  void message(int messagelevel, std::stringstream const &msg);
  
  /** Writes a warning **/ 
  void warning(const char buff[]);
  /** Writes a warning **/ 
  void warning(std::string const &msg);
  /** Writes a warning **/ 
  void warning(std::stringstream const &msg);

  /** Writes a error **/ 
  void error(const char buff[]);
  /** Writes a error **/ 
  void error(std::string const &msg);
  /** Writes a error **/ 
  void error(std::stringstream const &msg);
  
  /** Writes a MPI message **/ 
  void MPI(int level, const char buff[]);
  /** Writes a MPI message **/ 
  void MPI(int level, std::string const &msg);
  /** Writes a MPI message **/ 
  void MPI(int level, std::stringstream const &msg);
  

  /** << operators **/
  Log_writer &operator()(int i) {current_level = i; return *this;};
  
  Log_writer &operator<<(int i);
  Log_writer &operator<<(unsigned int i);
  Log_writer &operator<<(long int i);
  Log_writer &operator<<(unsigned long int i);
  Log_writer &operator<<(long long int i);
  Log_writer &operator<<(unsigned long long int i);
  Log_writer &operator<<(double d);
  Log_writer &operator<<(char ch[]);
  Log_writer &operator<<(std::string str);
  // for std::endl
  Log_writer &operator<<(std::ostream& (*f)(std::ostream&) ); 


  /// Sets all message levels to level
  void set_messagelevel(int level);
  int get_messagelevel() { return main_level; }

  /** Set the message level for the coming messages:
  **/ 
  void set_current_messagelevel(int level);

  void set_interactive(int i) { _interactive = i; }
  int  get_interactive()      { return _interactive; }

  void set_mpilevel(int level) { mpi_level = level; }
  int  get_mpilevel()          { return mpi_level; }

  /** Ask the user to proceed to the next correlation step. **/ 
  virtual void ask_continue();
private:
  virtual void write_message(const char buff[])=0;
  int main_level, current_level;
  int mpi_level, _interactive;
};

#endif /*LOG_WRITER_H_*/
