/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Log_writer.h>

Log_writer::Log_writer(Log_writer_buffer *str_buffer)
  : std::ostream(str_buffer), buffer(str_buffer)
{}

Log_writer::~Log_writer() {}

void Log_writer::set_messagelevel(int level) {
  buffer->set_messagelevel(level);
}

int
Log_writer::get_messagelevel() {
  return buffer->get_messagelevel();
}

void Log_writer::set_maxlevel(int level) {
  buffer->set_maxlevel(level);
}

int Log_writer::get_maxlevel() {
  return buffer->get_maxlevel();
}

Log_writer_buffer::Log_writer_buffer(int message_level, int buffer_size)
  : std::streambuf(), max_level(message_level), current_level(message_level)
{
  if (buffer_size) {
    char *ptr = new char[buffer_size];
    setp(ptr, ptr + buffer_size);
  } else {
    setp(0, 0);
  }
        
  setg(0, 0, 0);
}
Log_writer_buffer::~Log_writer_buffer()
{
  delete[] pbase();
}

void 
Log_writer_buffer::set_messagelevel(int i) {
  sync();
  current_level = i;
}

int
Log_writer_buffer::get_messagelevel() {
  return current_level;
}

void Log_writer_buffer::set_maxlevel(int level) {
  max_level = level;
}

int Log_writer_buffer::get_maxlevel() {
  return max_level;
}
