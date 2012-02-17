/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Aard Keimpema <keimpema@jive.nl>, 2008
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef VLBA_READER_H
#define VLBA_READER_H

#include "input_data_format_reader.h"

#include <fstream>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "data_reader.h"
#include "vlba_header.h"
#include "control_parameters.h"


class VLBA_reader : public Input_data_format_reader {
  enum Debug_level {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };
public:
  typedef Input_data_format_reader::Data_frame            Data_frame;

  VLBA_reader(boost::shared_ptr<Data_reader> data_reader, Time ref_date_);
  virtual ~VLBA_reader();

  bool open_input_stream(Data_frame &data);
  Time goto_time(Data_frame &data, Time time);

  /// Get the current time
  Time get_current_time();

  /// Read another vlba-frame
  bool read_new_block(Data_frame &data);

  Time time_between_headers() {
    SFXC_ASSERT(data_rate() % (N*SIZE_MK5A_FRAME) == 0);
    return time_between_headers_;
  }

  bool eof();

  void set_parameters(const Input_node_parameters &input_node_param);

  size_t bytes_per_input_word() const {
    return N;
  }
  size_t size_data_block() const {
    return SIZE_VLBA_FRAME*N;
  }

private:
  // format a time in miliseconds
  std::string time_to_string(int64_t time);

  // checking the header:
  bool check_time_stamp(VLBA_header &header);
  bool check_track_bit_statistics(Data_frame &data);
  // Find initial VLBA header
  bool find_start_of_header(Data_frame &data);
  // Resync header if there is mising data in the input stream
  bool resync_header(Data_frame &data);

  void set_data_frame_info(Data_frame &data);

  TRANSPORT_TYPE get_transport_type() const{
    return VLBA;
  }
private:
  // Time information
  int start_day_; // start date of data(mod Julian day%1000)
  int current_jday; // current modified julian day (to full precision)
  int ref_jday;
//  int64_t us_per_day;
  // start time and current time in micro seconds
  // start time is used to check the data rate
  Time start_time_, current_time_;
  // For testing
  Debug_level debug_level_;
  int block_count_;

  int data_rate() const;
  Time time_between_headers_;

  VLBA_header header;
  std::vector<unsigned char>  buf_header;

public:
  int DATA_RATE_;
  int N;
};

inline Time VLBA_reader::get_current_time() {
    return current_time_ - offset;
}
#endif // VLBA_READER_H
