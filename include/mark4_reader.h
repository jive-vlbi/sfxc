/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef MARK4_READER_H
#define MARK4_READER_H

#include <fstream>
#include "mark4_header.h"

template <class Type>
class Mark4_reader {
  enum Debug_level {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };
public:

  typedef Mark4_header<Type>        Header;

  // char buffer[SIZE_MK4_FRAME] contains the beginning of a mark4-frame
  Mark4_reader(Data_reader *data_reader,
               char *buffer,
               Type *mark4_block);
  virtual ~Mark4_reader();

  /// Time in miliseconds
  int32_t goto_time(int32_t time,
                    Type *mark4_block);

  /// Get the current time in miliseconds
  int32_t get_current_time();

  /// Read another mark4-frame
  bool read_new_block(Type *mark4_block);

  /// Get track information from a mark4 header
  std::vector< std::vector<int> >
  get_tracks(Input_node_parameters &input_node_param, Type *mark4_block);

private:
  // format a time in miliseconds
  std::string time_to_string(int32_t time);

  // checking the header:
  bool check_time_stamp(Header &header);
  bool check_track_bit_statistics(Type *mark4_block);
private:
  // Data reader: input stream
  Data_reader        *data_reader_;

  // Time information
  int start_day_, start_time_;
  int32_t current_time_;

  // For testing
  Debug_level debug_level_;
  int block_count_;
};

// Implementation

template <class Type>
Mark4_reader<Type>::
Mark4_reader(Data_reader *data_reader,
             char *buffer,
             Type *mark4_block)
    : data_reader_(data_reader),
    debug_level_(CHECK_PERIODIC_HEADERS),
block_count_(0) {
  // fill the first mark4 block
  memcpy(mark4_block, buffer, SIZE_MK4_FRAME*sizeof(char));
  int size = SIZE_MK4_FRAME*(sizeof(Type) - sizeof(char));
  int read = data_reader->get_bytes(size,
                                    ((char *)mark4_block) + SIZE_MK4_FRAME*sizeof(char));
  assert(read == size);

  Header header;
  header.set_header(mark4_block);
  header.check_header();
  start_day_ = header.day(0);
  start_time_ = header.get_time_in_ms(0);
  current_time_ = start_time_;
}

template <class Type>
Mark4_reader<Type>::
~Mark4_reader() {
  assert(data_reader_ != NULL);
  delete data_reader_;
}

template <class Type>
int32_t
Mark4_reader<Type>::
goto_time(int32_t time,
          Type *mark4_block) {
  if (time < get_current_time()) {
    std::cout << "time in past, current time is: "
    << time_to_string(get_current_time()) << std::endl;
    std::cout << "            requested time is: "
    << time_to_string(time) << std::endl;
    return get_current_time();
  } else if (time == get_current_time()) {
    return time;
  }

  size_t read_n_bytes =
    (time-get_current_time())*MARK4_TRACK_BIT_RATE*sizeof(Type)/1000 -
    SIZE_MK4_FRAME*sizeof(Type);

  // Read an integer number of frames
  assert(read_n_bytes %(SIZE_MK4_FRAME*sizeof(Type))==0);

  size_t result = data_reader_->get_bytes(read_n_bytes,NULL);
  if (result != read_n_bytes) {
    assert(false);
    return get_current_time();
  }

  // Need to read the data to check the header
  if (!read_new_block(mark4_block)) {
    assert(false);
    return get_current_time();
  }

  if (get_current_time() != time) {
    DEBUG_MSG("time:        " << time);
    DEBUG_MSG("current time: " << get_current_time());
    assert(get_current_time() == time);
  }

  return get_current_time();
}

template <class Type>
int32_t
Mark4_reader<Type>::
get_current_time() {
  return current_time_;
}

template <class Type>
std::string
Mark4_reader<Type>::
time_to_string(int32_t time) {
  int milisecond = time % 1000;
  time /= 1000;
  int second = time % 60;
  time /= 60;
  int minute = time % 60;
  time /= 60;
  int hour = time % 24;
  time /= 24;
  int day = time;

  char time_str[40];
  snprintf(time_str,40, "%03dd%02dh%02dm%02ds%03dms",
           day, hour, minute, second, milisecond);
  return std::string(time_str);

}

template <class Type>
bool
Mark4_reader<Type>::
read_new_block(Type *mark4_block) {
  int result = data_reader_->get_bytes(SIZE_MK4_FRAME*sizeof(Type),
                                       (char *)mark4_block)/sizeof(Type);

  if (result > (int)(160*sizeof(Type))) {
    // at least we read the complete header. Check it
    Header header;
    header.set_header(mark4_block);
    current_time_ = header.get_time_in_ms(0);

    if (debug_level_ >= CHECK_PERIODIC_HEADERS) {
      if ((debug_level_ >= CHECK_ALL_HEADERS) ||
          ((++block_count_ % 100) == 0)) {
        header.check_header();
        check_time_stamp(header);
        if (debug_level_ >= CHECK_BIT_STATISTICS) {
          if (!check_track_bit_statistics(mark4_block)) {
            std::cout << "Track bit statistics are off." << std::endl;
          }
        }
      }
    }

    if (result != SIZE_MK4_FRAME) {
      DEBUG_MSG("Didn't read a complete frame " << result);
      return false;
    }
  }
  return true;
}


template <class Type>
bool
Mark4_reader<Type>::check_time_stamp(Header &header) {
  int64_t militime = header.get_time_in_ms(0);
  int64_t delta_time =
    (header.day(0)-start_day_)*24*60*60*1000000 +
    militime*1000 + header.microsecond(0, militime)
    - start_time_;

  if (delta_time <= 0) {
    DEBUG_MSG("delta_time: " << delta_time)
    assert(delta_time > 0);
  }
  int64_t computed_TBR =
    (data_reader_->data_counter()*1000000/(sizeof(Type)*delta_time));

  if (computed_TBR != MARK4_TRACK_BIT_RATE) {
    return false;
  }
  return true;
}

template <class Type>
bool
Mark4_reader<Type>::check_track_bit_statistics(Type *mark4_block) {
  DEBUG_MSG(__PRETTY_FUNCTION__);
  double track_bit_statistics[sizeof(Type)*8];
  for (size_t track=0; track<sizeof(Type)*8; track++) {
    track_bit_statistics[track]=0;
  }

  for (int i=160; i<SIZE_MK4_FRAME; i++) {
    for (size_t track=0; track<sizeof(Type)*8; track++) {
      track_bit_statistics[track] += (mark4_block[i] >> track) &1;
    }
  }

  for (size_t track=0; track<sizeof(Type)*8; track++) {
    track_bit_statistics[track] /= SIZE_MK4_FRAME;
    if ((track_bit_statistics[track] < .45) ||
        (track_bit_statistics[track] > .55)) {
      return false;
    }
  }
  return true;
}


template <class Type>
std::vector< std::vector<int> >
Mark4_reader<Type>::get_tracks(Input_node_parameters &input_node_param,
                               Type *mark4_block) {
  Header header;
  header.set_header(mark4_block);
  assert(header.check_header());

  std::vector< std::vector<int> > result;

  result.resize(input_node_param.channels.size());
  int curr_channel =0;
  // Store a list of tracks: first magnitude (optional), then sign
  for (Input_node_parameters::Channel_const_iterator channel =
         input_node_param.channels.begin();
       channel != input_node_param.channels.end(); channel++, curr_channel++) {
    result[curr_channel].resize(channel->second.bits_per_sample() *
                                channel->second.sign_tracks.size());

    int track =0;
    for (size_t i=0; i<channel->second.sign_tracks.size(); i++) {
      result[curr_channel][track] =
        header.find_track(channel->second.sign_headstack-1,
                          channel->second.sign_tracks[i]);
      assert(header.headstack(result[curr_channel][track]) ==
             channel->second.sign_headstack-1);
      assert(header.track(result[curr_channel][track]) ==
             channel->second.sign_tracks[i]);
      track++;
      if (channel->second.bits_per_sample() == 2) {
        result[curr_channel][track] =
          header.find_track(channel->second.magn_headstack-1,
                            channel->second.magn_tracks[i]);
        assert(header.headstack(result[curr_channel][track]) ==
               channel->second.magn_headstack-1);
        assert(header.track(result[curr_channel][track]) ==
               channel->second.magn_tracks[i]);
        track++;
      }
    }
  }
  return result;
}

#endif // MARK4_READER_H
