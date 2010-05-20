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
                int N,
                Data_frame &data,
                int ref_year_,
                int ref_day_);
  virtual ~Mark5a_reader();

  /// Time in microseconds
  /// Changed the order of the arguments when I changed from miliseconds to microseconds
  int64_t goto_time(Data_frame &data, int64_t us_time);

  /// Get the current time in microseconds
  int64_t get_current_time();

  /// Read another mark5a-frame
  bool read_new_block(Data_frame &data);

  /// Get track information from a mark5a header
  std::vector< std::vector<int> > get_tracks(const Input_node_parameters &input_node_param,
                                             Data_frame &data);
  std::vector< std::vector<int> > get_standard_track_mapping(const Input_node_parameters &input_node_param,
                                                             Data_frame &data);

  int time_between_headers() {
    SFXC_ASSERT(data_rate() % (N*SIZE_MK5A_FRAME*8) == 0);
    return N * 8 * SIZE_MK5A_FRAME * 1000000LL / data_rate();
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
  // format a time in miliseconds
  std::string time_to_string(int64_t time);

  // checking the header:
  bool check_time_stamp(Mark5a_header &header);
  bool check_track_bit_statistics(Data_frame &data);

  // Resync header if there is mising data in the input stream
  bool resync_header(Data_frame &data);

  // Convert time read from input stream to time relative to midnight on the reference day
  int64_t correct_raw_time(int64_t raw_time);

  void set_data_frame_info(Data_frame &data);
private:
  // Time information
  int start_day_, current_day_;
  int ref_day, ref_year;
  int64_t us_per_day;
  // start time and current time in microseconds
  // start time is used to check the data rate
  int64_t start_time_, current_time_;

  // For testing
  Debug_level debug_level_;
  int block_count_;

  int data_rate() const;

public:
  int DATA_RATE_;
  const int N;
};



/** Returns a mark5a reader based on the headers in the data stream
 **/
Mark5a_reader *
get_mark5a_reader(boost::shared_ptr<Data_reader> reader,
                  Mark5a_reader::Data_frame &data, int ref_year, int ref_day);

int find_start_of_header(boost::shared_ptr<Data_reader> reader,
                         Mark5a_reader::Data_frame &data);


#endif // MARK5A_READER_H
