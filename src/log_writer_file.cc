/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include <fstream>
#include <assert.h>

#include "utils.h"
#include "log_writer_file.h"

class Log_writer_file_buffer : public Log_writer_buffer {
public:
  Log_writer_file_buffer(char *filename,
                         int messagelevel,
                         int buffer_size=160);
  ~Log_writer_file_buffer();

private:
  void put_buffer(void);
  void put_char(int);

protected:
  int overflow(int);
  int sync();

  std::ofstream out;
};

Log_writer_file::Log_writer_file(char *filename,
                                 int messagelevel)
    : Log_writer(new Log_writer_file_buffer(filename, messagelevel)) {}

// Buffer
Log_writer_file_buffer::Log_writer_file_buffer(char *filename,
    int message_level,
    int buffer_size)
    : Log_writer_buffer(message_level, buffer_size),
    out(filename+7) {
  assert(strncmp(filename, "file://", 7)==0);
  assert(out.is_open());
}

Log_writer_file_buffer::~Log_writer_file_buffer() {
  sync();
}

int Log_writer_file_buffer::overflow(int c) {
  put_buffer();

  if (c != EOF)
    if (pbase() == epptr())
      put_char(c);
    else
      sputc(c);

  return 0;
}

int Log_writer_file_buffer::sync() {
  put_buffer();
  return 0;
}

void Log_writer_file_buffer::put_char(int chr) {
  if (current_level <= max_level)
    out << char(chr);
}
void Log_writer_file_buffer::put_buffer() {
  if (pbase() != pptr()) {
    int     len = (pptr() - pbase());
    char    *buffer = new char[len + 1];

    strncpy(buffer, pbase(), len);
    buffer[len] = 0;

    if (current_level <= max_level)
      out << buffer << std::flush;

    setp(pbase(), epptr());
    delete [] buffer;
  }
}
