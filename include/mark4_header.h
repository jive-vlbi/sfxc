/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef MARK4_HEADER_H
#define MARK4_HEADER_H

#include <assert.h>

#include "utils.h"

/** Mark IV frame header in byte chunks, without parity bits. */
class Mark4_header  {
public:
  // N is the size of one input word in bytes
  Mark4_header(int N);

  void set_header(unsigned char *header);
  bool check_header();

  int nTracks();

  int headstack(int track);
  int track(int track);
  bool is_sign(int track);
  bool is_magn(int track);

  int year(int track);
  int day(int track);
  int hour(int track);
  int minute(int track);
  int second(int track);
  int milisecond(int track);
  // Track is not used, just to make the function idiot proof ( me :) )
  int microsecond(int track, int milisecond);

  int find_track(int headstack, int track);

  int get_time_in_ms(int track);
  int64_t get_time_in_us(int track);
  std::string get_time_str(int track);

  bool checkCRC();
  void recomputeCRC();

  bool is_valid();

  static const int microsecond_offset[];

  Log_writer &print_binary_header(Log_writer &writer);
private:
  template <class Type>
  void crc12(Type *crcBlock, Type *data, int datawords);

  // Binary coded decimal:
  int BCD(int word, unsigned int track);

  int get_bit(int word, int track) {
    return (header_[word*N + track/8] >> track%8)&1;
  }
private:
  unsigned char* header_;

  static const char *header_map[];

  // Number of bytes per data word (#tracks/8)
  const unsigned int N;
};

#endif // MARK4_HEADER_H
