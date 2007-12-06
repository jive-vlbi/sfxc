#ifndef MARK4_READER_H
#define MARK4_READER_H

#include "mark4_header.h"

template <class Type>
class Mark4_reader
{
  enum Debug_level {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };
public:
  // char buffer[SIZE_MK4_FRAME] contains the beginning of a mark4-frame
  Mark4_reader(Data_reader *data_reader, char *buffer);
  virtual ~Mark4_reader();

  /// Time in miliseconds
  int32_t goto_time(int32_t time);
  
  /// Get the current time in miliseconds
  int32_t get_current_time();

  /// Read another mark4-frame
  bool read_new_block();
private:
  // format a time in miliseconds
  std::string time_to_string(int32_t time);

  // checking the header:
  bool check_time_stamp();
  bool check_track_bit_statistics();
private:
  Type               block[SIZE_MK4_FRAME];
  Data_reader        *data_reader;
  Mark4_header<Type> mark4_header;

  // For testing
  Debug_level debug_level;
  int block_count;
  // start time and day in miliseconds
  int start_day, start_time;
};

// Implementation

template <class Type>
Mark4_reader<Type>::
Mark4_reader(Data_reader *data_reader, char *buffer) 
  : data_reader(data_reader), debug_level(CHECK_PERIODIC_HEADERS),
    block_count(0)
{
  // fill the first mark4 block
  memcpy(block, buffer, SIZE_MK4_FRAME*sizeof(char));
  int size = SIZE_MK4_FRAME*(sizeof(Type) - sizeof(char));
  int read = data_reader->get_bytes(size,
                                    ((char *)block) + SIZE_MK4_FRAME*sizeof(char));
  assert(read == size);

  mark4_header.set_header(block);
  mark4_header.check_header();
  start_day = mark4_header.day(0);
  start_time = get_current_time();
}

template <class Type>
Mark4_reader<Type>::
~Mark4_reader() {
  assert(data_reader != NULL);
  delete data_reader;
}

template <class Type>
int32_t
Mark4_reader<Type>::
goto_time(int32_t time) {
  int32_t current_time = get_current_time();

  if (time < current_time) {
    std::cout << "time in past, current time is: " 
              << time_to_string(current_time) << std::endl;
    std::cout << "            requested time is: " 
              << time_to_string(time) << std::endl;
    return -1;
  } else if (time == current_time) {
    return 0;
  }

  size_t read_n_bytes = 
    (time-current_time)*MARK4_TRACK_BIT_RATE*sizeof(Type)/1000 -
    SIZE_MK4_FRAME*sizeof(Type);
  
  assert(read_n_bytes > 0);
  // Read an integer number of frames
  assert(read_n_bytes %(SIZE_MK4_FRAME*sizeof(Type))==0);

  size_t result = data_reader->get_bytes(read_n_bytes,NULL);
  if (result != read_n_bytes) {
    assert(false);
    return get_current_time();
  }

  // Need to read the data to check the header
  if (!read_new_block()) {
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
  return mark4_header.get_time_in_ms(0);
}

template <class Type>
std::string
Mark4_reader<Type>::
time_to_string(int32_t time) {
  int milisecond = time % 1000; time /= 1000;
  int second = time % 60; time /= 60;
  int minute = time % 60; time /= 60;
  int hour = time % 24; time /= 24;
  int day = time;

  char time_str[40];
  snprintf(time_str,40, "%03dd%02dh%02dm%02ds%03dms",
	   day, hour, minute, second, milisecond);
  return std::string(time_str);
  
}

template <class Type>
bool
Mark4_reader<Type>::
read_new_block() {
  int result = data_reader->get_bytes(SIZE_MK4_FRAME*sizeof(Type),
                                      (char *)block)/sizeof(Type);
  if (result != SIZE_MK4_FRAME) {
    return false;
  }

  if (debug_level >= CHECK_PERIODIC_HEADERS) {
    if ((debug_level >= CHECK_ALL_HEADERS) ||
        ((++block_count % 100) == 0)) {
      mark4_header.check_header();
      check_time_stamp();
      if (debug_level >= CHECK_BIT_STATISTICS) {
        if (!check_track_bit_statistics()) {
          std::cout << "Track bit statistics are off." << std::endl;
        }
      }
    }
  }

  return true;
}


template <class Type>
bool
Mark4_reader<Type>::check_time_stamp() {
  DEBUG_MSG(__PRETTY_FUNCTION__);
  int64_t militime = mark4_header.get_time_in_ms(0);
  int64_t delta_time = 
    (mark4_header.day(0)-start_day)*24*60*60*1000000 + 
    militime*1000 + mark4_header.microsecond(0, militime)
    - start_time;

  if (delta_time <= 0) {
    DEBUG_MSG("delta_time: " << delta_time)
    assert(delta_time > 0);
  }
  int64_t computed_TBR =
    (data_reader->data_counter()*1000000/(sizeof(Type)*delta_time));
  
  if (computed_TBR != MARK4_TRACK_BIT_RATE) {
    return false;
  }
  return true;
}

template <class Type>
bool
Mark4_reader<Type>::check_track_bit_statistics() {
  DEBUG_MSG(__PRETTY_FUNCTION__);
  double track_bit_statistics[sizeof(Type)*8];
  for (size_t track=0; track<sizeof(Type)*8; track++) {
    track_bit_statistics[track]=0;
  }
  
  for (int i=160; i<SIZE_MK4_FRAME; i++) {
    for (size_t track=0; track<sizeof(Type)*8; track++) {
      track_bit_statistics[track] += (block[i] >> track) &1;
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

#endif // MARK4_READER_H
