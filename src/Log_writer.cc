#include <Log_writer.h>

char* itoa(int value, char* result, int base ) {
  // check that the base if valid
  if (base < 2 || base > 16) { *result = 0; return result; }
  
  char* out = result;
  int quotient = value;

  do {
    *out = "0123456789abcdef"[ std::abs( quotient % base ) ];
    ++out;
    quotient /= base;
  } while ( quotient );
  
  // Only apply negative sign for base 10
  if ( value < 0 && base == 10) *out++ = '-';
  
  std::reverse( result, out );
  *out = 0;
  
  return result;
}

Log_writer::Log_writer(int messagelevel, bool interactive)
 : main_level(messagelevel), current_level(0), mpi_level(main_level), _interactive(interactive) 
{}

Log_writer::~Log_writer() {}

void Log_writer::message(int messagelevel, const char buff[]) {
  if (main_level >= messagelevel) {
    write_message(buff);
    write_message("\n");
  } 
}

void Log_writer::message(int messagelevel, std::string const &msg) {
  if (main_level >= messagelevel) {
    write_message(msg.c_str());
    write_message("\n");
  } 
}

void Log_writer::message(int messagelevel, std::stringstream const &msg) {
  if (main_level >= messagelevel) {
    write_message(msg.str().c_str());
    write_message("\n");
  } 
}

/* WARNING */
void Log_writer::warning(const char buff[]) {
  write_message("WARNING: ");
  write_message(buff);
  write_message("\n");
}
void Log_writer::warning(std::string const &msg) {
  write_message("WARNING: ");
  write_message(msg.c_str());
  write_message("\n");
}
void Log_writer::warning(std::stringstream const &msg) {
  write_message("WARNING: ");
  write_message(msg.str().c_str());
  write_message("\n");
}

/* ERROR */
void Log_writer::error(const char buff[]) {
  write_message("ERROR: ");
  write_message(buff);
  write_message("\n");
}
void Log_writer::error(std::string const &msg) {
  write_message("ERROR: ");
  write_message(msg.c_str());
  write_message("\n");
}
void Log_writer::error(std::stringstream const &msg) {
  write_message("ERROR: ");
  write_message(msg.str().c_str());
  write_message("\n");
}

/* MPI */
void Log_writer::MPI(int level, const char buff[]) {
  if (mpi_level >= level) {
    write_message("MPI: ");
    write_message(buff);
    write_message("\n");
  }
}
void Log_writer::MPI(int level, std::string const &msg) {
  if (mpi_level >= level) {
    write_message("MPI: ");
    write_message(msg.c_str());
    write_message("\n");
  }
}
void Log_writer::MPI(int level, std::stringstream const &msg) {
  if (mpi_level >= level) {
    write_message("MPI: ");
    write_message(msg.str().c_str());
    write_message("\n");
  }
}

void Log_writer::set_current_messagelevel(int level) {
  current_level = level;
}

void Log_writer::set_messagelevel(int level) {
  main_level = level;
  current_level = level;
  mpi_level = level;
}

Log_writer &Log_writer::operator<<(int i) {
  if (main_level >= current_level) {
    char str[20];
    sprintf(str, "%d", i);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(unsigned int i) {
  if (main_level >= current_level) {
    char str[20];
    sprintf(str, "%u", i);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(long int i) {
  if (main_level >= current_level) {
    char str[20];
    snprintf(str,20, "%ld", i);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(unsigned long int i) {
  if (main_level >= current_level) {
    char str[20];
    snprintf(str,20, "%lu", i);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(long long int i) {
  if (main_level >= current_level) {
    char str[20];
    snprintf(str,20, "%lld", i);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(unsigned long long int i) {
  if (main_level >= current_level) {
    char str[20];
    snprintf(str,20, "%llu", i);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(double d) {
  if (main_level >= current_level) {
    char str[20];
    sprintf(str, "%f", d);
    write_message(str);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(char ch[]) {
  if (main_level >= current_level) {
    write_message(ch);
  }
  return *this;
}

Log_writer &Log_writer::operator<<(std::string str) {
  if (main_level >= current_level) {
    write_message(str.c_str());
  }
  return *this;
}

Log_writer &
Log_writer::operator<<(std::ostream& (*f)(std::ostream&)) {
  if (main_level >= current_level) {
    write_message("\n");
  }
  return *this;
} 



void Log_writer::ask_continue() {}
