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

#include <fstream>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "data_reader.h"
#include "control_parameters.h"

class Mark5b_reader {
  enum Debug_level {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };
public:

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
                unsigned char *buffer);
  virtual ~Mark5b_reader();

  /// Time in microseconds
  /// Changed the order of the arguments when I changed from miliseconds to microseconds
  int64_t goto_time(unsigned char *mark5b_block,
                    int64_t us_time);

  /// Get the current time in microseconds
  int64_t get_current_time();

  /// Read another mark5b-frame
  bool read_new_block(unsigned char *mark5b_block);

  bool eof();

  void set_track_bit_rate(int tbr);
  int time_between_headers();

  std::vector< std::vector<int> >
  get_tracks(const Input_node_parameters &input_node_param);

private:
  // Data reader: input stream
  // The file pointer is always after a header, but before the data
  boost::shared_ptr<Data_reader> data_reader_;

  // Time information
  int start_day_;
  // start time and current time in miliseconds
  // start time is used to check the data rate
  int64_t start_time_, current_time_;

  // For testing
  Debug_level debug_level_;

  // Current header is the first header read in read_new_block
  // tmp_header is used for the other blocks
  // so that the header points to the time of the first sample
  Header current_header, tmp_header;

  int time_between_headers_;
};

std::ostream &operator<<(std::ostream &out,
                         const Mark5b_reader::Header &header);

#endif // MARK5B_READER_H
