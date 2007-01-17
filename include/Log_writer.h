#ifndef LOG_WRITER_H_
#define LOG_WRITER_H_

#include <string>
#include <sstream>
#include <iostream>
#include "types.h"

using namespace std;

class Log_writer {
public:
  Log_writer(int messagelevel);
  
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
  
  /** Set the message level for the coming messages:
  **/ 
  void set_current_messagelevel(int level);

  /** << operators **/
  Log_writer &operator()(int i) {set_messagelevel(i); return *this;};
  
  Log_writer &operator<<(INT32 i);
  Log_writer &operator<<(UINT32 i);
  Log_writer &operator<<(INT64 i);
  Log_writer &operator<<(UINT64 i);
  Log_writer &operator<<(double d);
  Log_writer &operator<<(char ch[]);
  Log_writer &operator<<(std::string str);
  // for std::endl
  Log_writer &operator<<(std::ostream& (*f)(std::ostream&) ); 

  void set_messagelevel(int level) { _level = level; }
  int  get_messagelevel()          { return _level; }

  /** Ask the user to proceed to the next correlation step.
  **/ 
  virtual void ask_continue();
private:
  virtual void write_message(const char buff[])=0;
  int _level, current_level;
};

#endif /*LOG_WRITER_H_*/
