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

class Log_writer_buffer : public std::streambuf {
public:
  Log_writer_buffer(int max_level,
                    int buffer_size=160);
  ~Log_writer_buffer();

  void set_messagelevel(int i);
  int  get_messagelevel();

  void set_maxlevel(int level);
  int get_maxlevel();

protected:
  int max_level, current_level;
  virtual int sync()=0;

};


class Log_writer : public std::ostream {
public:
  Log_writer(Log_writer_buffer *str_buffer);
  virtual ~Log_writer();

  /// Sets message level
  Log_writer &operator()(int i) {
    set_messagelevel(i);
    return *this;
  };
  void set_messagelevel(int level);
  int  get_messagelevel();

  void set_maxlevel(int level);
  int  get_maxlevel();
private:
  Log_writer_buffer *buffer;
};

/// Conversion of an integer-type to a character.
template <class T>
char* itoa(T value, char* result, int base ) {
  // check that the base if valid
  if (base < 2 || base > 16) {
    *result = 0;
    return result;
  }

  char* out = result;
  T quotient = value;
  int sign = 1;

  if (quotient != (T)abs(quotient)) {
    sign = -1;
    quotient = - quotient;
  }

  do {
    *out = "0123456789abcdef"[ quotient % base ];
    ++out;
    quotient /= base;
  } while ( quotient );

  // Only apply negative sign for base 10
  if ( (sign == -1) && (base == 10) ) *out++ = '-';

  std::reverse( result, out );
  *out = 0;

  return result;
}


#endif /*LOG_WRITER_H_*/
