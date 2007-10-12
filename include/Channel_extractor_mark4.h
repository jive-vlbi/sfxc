/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef CHANNEL_EXTRACTOR_MARK4_H_
#define CHANNEL_EXTRACTOR_MARK4_H_

#include <Channel_extractor.h>
#include <Data_reader.h>
#include <Data_writer.h>

#include <boost/shared_ptr.hpp>
#include <Log_writer.h>

#include <Control_parameters.h>
#include <vector>

// Templated by the type of the element from which the samples are extracted
// Either int32_t (n_head_stacks == 1) or int64_t (n_head_stacks == 2)
template <class T>
class Channel_extractor_mark4_implementation;

class Channel_extractor_mark4 : public Channel_extractor
{
public:
  enum Debug_level {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };

  Channel_extractor_mark4(boost::shared_ptr<Data_reader> reader, 
                          bool insert_random_headers_,
                          Debug_level debug_level = CHECK_ALL_HEADERS);

  bool set_track_parameters(const Track_parameters &parameters);
  
  int goto_time(int64_t time);
  int64_t get_current_time();

  /// Gets the data for the channel from the current mark4 block.
  /// Buff contains a buffer for every channel or NULL if no output is requested.
  size_t get_bytes(std::vector< char* > &buff);
  
  // Returns 0 if ok
  int goto_next_block();
  
  // returns the number of bytes for each channel returned by one get_samples
  // call
  int number_of_bytes_per_block();
  
  bool eof();

  void print_header(Log_writer &writer, int track=0);

  int n_tracks();
  int headstack(int track);
  int track(int track);
  
  int n_channels();
  int n_tracks(int channel);

  int track_bit_rate() const;
  int bit_rate(int channel) const;
  
private:
  int find_header(char *buffer, boost::shared_ptr<Data_reader> reader);

  
  /** The number of tracks, determined from the data **/
  int total_tracks;
  
  // Different implementations based on the number of head stacks:
  Channel_extractor_mark4_implementation<uint8_t>     *ch_extractor_8_tracks;
  Channel_extractor_mark4_implementation<uint16_t>    *ch_extractor_16_tracks;
  Channel_extractor_mark4_implementation<uint32_t>    *ch_extractor_32_tracks;
  Channel_extractor_mark4_implementation<uint64_t>    *ch_extractor_64_tracks;
};

#endif /*CHANNEL_EXTRACTOR_MARK4_H_*/
