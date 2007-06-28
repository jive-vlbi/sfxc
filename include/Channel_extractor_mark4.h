/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef MARK4_CHANNEL_EXTRACTOR_H_
#define MARK4_CHANNEL_EXTRACTOR_H_

#include <Channel_extractor.h>
#include <Data_writer.h>

#include <staPrms.h>

#include <vector>

// Templated by the type of the element from which the samples are extracted
// Either INT32 (n_head_stacks == 1) or INT64 (n_head_stacks == 2)
template <class T>
class Channel_extractor_mark4_implementation;

class Channel_extractor_mark4 : public Channel_extractor
{
public:
  enum DEBUG_LEVEL {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };

  Channel_extractor_mark4(Data_reader &reader, 
                          StaP &StaPrms, 
                          bool insert_random_headers_,
                          DEBUG_LEVEL debug_level = CHECK_PERIODIC_HEADERS);
  
  int goto_time(INT64 time);
  INT64 get_current_time();

  /** Returns a number of samples, one sample per character. **/
  size_t get_samples(size_t nSamples, double *bit_samples, 
                     const double *val_array);

  bool eof();

  void print_header(Log_writer &writer, int track=0);

  int number_of_tracks();
  int headstack(int track);
  int track(int track);
private:
  int find_header(char *buffer, Data_reader &reader);

  size_t do_get_bytes(size_t nBytes, char *buff);
  
  /** The number of tracks, determined from the data **/
  int n_tracks;
  
  // Different implementations based on the number of head stacks:
  Channel_extractor_mark4_implementation<uint8_t>     *ch_extractor_8_tracks;
  Channel_extractor_mark4_implementation<uint16_t>    *ch_extractor_16_tracks;
  Channel_extractor_mark4_implementation<uint32_t>    *ch_extractor_32_tracks;
  Channel_extractor_mark4_implementation<uint64_t>    *ch_extractor_64_tracks;
};

#endif /*MARK4_CHANNEL_EXTRACTOR_H_*/
