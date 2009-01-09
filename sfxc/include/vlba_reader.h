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

  VLBA_reader(boost::shared_ptr<Data_reader> data_reader,
                int N,
                Data_frame &data, Data_frame &header_, Data_frame &aux_header_);
  virtual ~VLBA_reader();

  /// Time in microseconds
  int64_t goto_time(Data_frame &data, int64_t us_time);

  /// Get the current time in microseconds
  int64_t get_current_time();

  /// Read another vlba-frame
  bool read_new_block(Data_frame &data);

  /// Get track information from a vlba header
  std::vector< std::vector<int> >
  get_tracks(const Input_node_parameters &input_node_param, 
             Data_frame &data);


  int time_between_headers() {
    SFXC_ASSERT(data_rate() % (N*SIZE_VLBA_FRAME) == 0);
    return data_rate() / (N*SIZE_VLBA_FRAME);
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

  // Convert time read from input stream to time relative to midnight on the reference day
  int64_t correct_raw_time(int64_t raw_time);

  void set_data_frame_info(Data_frame &data);
private:
  // Time information
  int start_day_; // start date of data(mod Julian day%1000)
  int ref_jday; //date relative to which times are calculated(mod. Julian day%1000)
  int64_t us_per_day;
  // start time and current time in micro seconds
  // start time is used to check the data rate
  int64_t start_time_, current_time_;
  // For testing
  Debug_level debug_level_;
  int block_count_;

  int data_rate() const;

  VLBA_header header;
  std::vector<unsigned char>  buf_header;
  std::vector<unsigned char>  buf_aux_header;

public:
  int DATA_RATE_;
  const int N;
};



/** Returns a vlba reader based on the headers in the data stream
 **/
VLBA_reader *
get_vlba_reader(boost::shared_ptr<Data_reader> reader,
                  VLBA_reader::Data_frame &data);

int find_start_of_vlba_header(boost::shared_ptr<Data_reader> reader,
                              VLBA_reader::Data_frame &data, 
                              VLBA_reader::Data_frame &header,
                              VLBA_reader::Data_frame &aux_header);

#endif // VLBA_READER_H
