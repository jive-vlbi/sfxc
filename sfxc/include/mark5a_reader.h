/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef MARK5A_READER_H
#define MARK5A_READER_H

#include "input_data_format_reader.h"

#include <fstream>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "data_reader.h"
#include "mark5a_header.h"
#include "control_parameters.h"
#include "correlator_time.h"

class Mark5a_reader : public Input_data_format_reader {
  enum Debug_level {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };
public:
  typedef Input_data_format_reader::Data_frame            Data_frame;

  Mark5a_reader(boost::shared_ptr<Data_reader> data_reader,
                Time ref_time_);
  virtual ~Mark5a_reader();

  bool open_input_stream(Data_frame &data);
  Time goto_time(Data_frame &data, Time us_time);

  /// Get the current time
  Time get_current_time();

  /// Read another mark5a-frame
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
    return SIZE_MK5A_FRAME*N;
  }

  TRANSPORT_TYPE get_transport_type() const{
    return MARK5A;
  }

private:
  bool find_start_of_header(boost::shared_ptr<Data_reader> reader, Data_frame &data);

  // format a time in miliseconds
  std::string time_to_string(int64_t time);

  // checking the header:
  bool check_time_stamp(Mark5a_header &header);
  bool check_track_bit_statistics(Data_frame &data);

  // Resync header if there is mising data in the input stream
  bool resync_header(Data_frame &data);

  void set_data_frame_info(Data_frame &data);
  void generate_track_mask();
private:
  // Time information
  int start_day_, current_day_, current_mjd_;
  int ref_year, ref_day;
  //int64_t us_per_day;
  // start time and current time in microseconds
  // start time is used to check the data rate
  Time start_time_, current_time_;
  // For testing
  Debug_level debug_level_;
  int block_count_;

  int track;    // track that is used to retrieve time stamp
  uint8_t mask; // used to mask out tracks using syncword search
  int data_rate() const;
  Time time_between_headers_;

public:
  int DATA_RATE_;
  int N;
};

inline Time Mark5a_reader::get_current_time() {
  return current_time_;
}

inline bool Mark5a_reader::eof() {
  return data_reader_->eof();
}

#endif // MARK5A_READER_H
