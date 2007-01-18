#include <Log_writer.h>

Log_writer::Log_writer(int messagelevel) : _level(messagelevel), current_level(0) {}

Log_writer::~Log_writer() {}

void Log_writer::message(int messagelevel, const char buff[]) {
  if (_level >= messagelevel) {
    write_message(buff);
    write_message("\n");
  } 
}

void Log_writer::message(int messagelevel, std::string const &msg) {
  if (_level >= messagelevel) {
    write_message(msg.c_str());
    write_message("\n");
  } 
}

void Log_writer::message(int messagelevel, std::stringstream const &msg) {
  if (_level >= messagelevel) {
    write_message(msg.str().c_str());
    write_message("\n");
  } 
}

void Log_writer::set_current_messagelevel(int level) {
  current_level = level;
}

Log_writer &Log_writer::operator<<(INT32 i) {
  if (_level >= current_level) {
    char str[20];
    sprintf(str, "%d", i);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(UINT32 i) {
  if (_level >= current_level) {
    char str[20];
    sprintf(str, "%ud", i);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(INT64 i) {
  if (_level >= current_level) {
    char str[20];
    snprintf(str,20, "%lld", i);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(UINT64 i) {
  if (_level >= current_level) {
    char str[20];
    snprintf(str,20, "%llud", i);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(double d) {
  if (_level >= current_level) {
    char str[20];
    sprintf(str, "%f", d);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(char ch[]) {
  if (_level >= current_level) {
    write_message(ch);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(std::string str) {
  if (_level >= current_level) {
    write_message(str.c_str());
  }
  return *this;
}

Log_writer &
Log_writer::operator<<(std::ostream& (*f)(std::ostream&)) {
  if (_level >= current_level) {
    write_message("\n");
  }
  return *this;
} 



void Log_writer::ask_continue() {}
