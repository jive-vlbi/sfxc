/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef MARK5B_READER_H
#define MARK5B_READER_H

#include "input_data_format_reader.h"

#include "data_reader.h"
#include "input_node_types.h"
#include "control_parameters.h"

#include <fstream>
#include <vector>
#include <boost/shared_ptr.hpp>

class Mark5b_reader : public Input_data_format_reader {
  enum Debug_level {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };
public:
  typedef Input_data_format_reader::Data_frame Data_frame;

  struct Header {
    uint32_t      syncword;
    uint32_t      frame_nr:15;
    uint8_t       tvg:1;
    uint16_t      user_specified;

    uint8_t sec5:4, sec4:4, sec3:4, sec2:4, sec1:4;
    uint8_t day3:4, day2:4, day1:4;
    uint16_t crc;
    uint8_t subsec4:4, subsec3:4, subsec2:4, subsec1:4;

    bool check() const;

    // Time in microseconds since midnight (approx)
    int64_t microseconds() const;

    // Time in secons since midnight (truncated time)
    int64_t seconds() const;

    // Julian day % 1000
    int julian_day() const;

  };

  Mark5b_reader(boost::shared_ptr<Data_reader> data_reader,
                Data_frame &data, Time ref_date);
  virtual ~Mark5b_reader();

  /// Time in microseconds
  /// Changed the order of the arguments when I changed from miliseconds to microseconds
  Time goto_time(Data_frame &data, Time time);

  /// Get the current time in microseconds
  Time get_current_time();

  /// Read another mark5b-frame
  bool read_new_block(Data_frame &data);

  bool resync_header(Data_frame &data);

  bool eof();

  Time time_between_headers();

  size_t bytes_per_input_word() const {
    return SIZE_MK5B_WORD;
  }

  size_t size_data_block() const {
    return bytes_per_input_word()*SIZE_MK5B_FRAME*N_MK5B_BLOCKS_TO_READ;
  }

  std::vector< std::vector<int> >
  get_tracks(const Input_node_parameters &input_node_param,
             Data_frame &data);

  void set_parameters(const Input_node_parameters &param);

  TRANSPORT_TYPE get_transport_type() const{
    return MARK5B;
  }

private:
  // Time information
  int start_day_; // start date of data(mod Julian day%1000)
  int current_jday; // current modified julian day (to full precision)
//  int64_t us_per_day;
  // start time and current time in miliseconds
  // start time is used to check the data rate
  Time start_time_, current_time_;

  int sample_rate;
  int nr_of_bitstreams;

  // For testing
  Debug_level debug_level_;

  // Current header is the first header read in read_new_block
  // tmp_header is used for the other blocks
  // so that the header points to the time of the first sample
  Header current_header, tmp_header;

  Time time_between_headers_;

  // The lookup table for the CRC-16 checks
  std::vector<uint16_t> crc_table;
  // generate the crc16 lookup table
  void gen_crc16();
  // Check the integrity of the current header
  bool check_header(Header &header);
};

std::ostream &operator<<(std::ostream &out,
                         const Mark5b_reader::Header &header);

#endif // MARK5B_READER_H
