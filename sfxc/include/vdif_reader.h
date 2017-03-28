/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef VDIF_READER_H
#define VDIF_READER_H

#include "input_data_format_reader.h"

#include "data_reader.h"
#include "input_node_types.h"
#include "control_parameters.h"

#include <fstream>
#include <vector>
#include <boost/shared_ptr.hpp>

// The number VDIF frames to be read is rounded to this number of bytes
#define VDIF_FRAME_BUFFER_SIZE    32128

class VDIF_reader : public Input_data_format_reader {
  enum Debug_level {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };
public:
  typedef Input_data_format_reader::Data_frame Data_frame;

  struct Header {
    // Word 0
    uint32_t      sec_from_epoch:30;
    uint8_t       legacy_mode:1, invalid:1;
    // Word 1
    uint32_t      dataframe_in_second:24;
    uint8_t       ref_epoch:6, unassiged:2;
    // Word 2
    uint32_t      dataframe_length:24;
    uint8_t       log2_nchan:5, version:3;
    // Word 3
    uint16_t      station_id:16, thread_id:10;
    uint8_t       bits_per_sample:5, data_type:1;
    // Word 4
    uint32_t      user_data1:24;
    uint8_t       edv:8;
    // Word 5-7
    uint32_t      user_data2,user_data3,user_data4;

    // Start of epoch in jday
    int32_t jday_epoch() const;
    // Header size in bytes (words 4-7 are not present in legacy mode)
    int32_t header_size() const;
    // Size of data inside data_frame in bytes
    int32_t data_size() const;
  };

  VDIF_reader(boost::shared_ptr<Data_reader> data_reader,
                Data_frame &data, Time ref_time);
  virtual ~VDIF_reader();

  bool open_input_stream(Data_frame &data);
  /// Time in microseconds
  /// Changed the order of the arguments when I changed from miliseconds to microseconds
  Time goto_time(Data_frame &data, Time us_time);

  /// Get the current time in microseconds
  Time get_current_time();

  /// Read another VDIF-frame
  bool read_new_block(Data_frame &data);

  bool eof();

  Time time_between_headers();

  void print_header();
  size_t size_data_block() const {
    return vdif_frames_per_block * first_header.data_size();
  }

  void set_parameters(const Input_node_parameters &param);

  size_t bytes_per_input_word() const {
    // Round up to the nearest 32-bit word boundary
    return ((bits_per_complete_sample + 31) / 32) * 4;
  }

  TRANSPORT_TYPE get_transport_type() const{
    return VDIF;
  }

private:
  // Time information
  int ref_jday; //date relative to which times are calculated(mod Julian day)
  // current time in microseconds
  Time current_time_;
  Time time_between_headers_;
  int bits_per_complete_sample;
  int vdif_frames_per_block;
  double sample_rate;
  // For testing
  Debug_level debug_level_;

  // We keep a copy of the first header to validate subsequent headers.
  Header first_header, current_header;
  bool first_header_seen;

  // Convert time read from input stream to time relative to midnight on the reference day
  int64_t correct_raw_time(int64_t raw_time);

  // Mapping between thread IDs and channel numbers.
  std::map<int, int> thread_map;
};

inline Time 
VDIF_reader::time_between_headers() {
 return time_between_headers_;
}

inline int32_t 
VDIF_reader::Header::header_size() const{
  return 16+16*(1-legacy_mode);
}

inline int32_t 
VDIF_reader::Header::data_size() const{
  return 8*dataframe_length-header_size();
}

std::ostream &operator<<(std::ostream &out,
                         const VDIF_reader::Header &header);

#endif // VDIF_READER_H
