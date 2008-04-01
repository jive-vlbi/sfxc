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
#include <vector>
#include <boost/shared_ptr.hpp>

#include "data_reader.h"
#include "mark4_header.h"
#include "control_parameters.h"

class Mark4_reader_interface {
public:
  Mark4_reader_interface() {}
  virtual ~Mark4_reader_interface() {}

  /// Time in microseconds
  /// Changed the order of the arguments when I changed from miliseconds to microseconds
  virtual int64_t goto_time(unsigned char *data_block, int64_t us_time) = 0;

  /// Get the current time in microseconds
  virtual int64_t get_current_time() = 0;

  /// Read another mark4-frame
  virtual bool read_new_block(unsigned char *data_block) = 0;

  // The time between two headers in microseconds
  virtual int time_between_headers() = 0;
};

/** Returns a mark4 reader based on the headers in the data stream
 **/
Mark4_reader_interface *
get_mark4_reader(boost::shared_ptr<Data_reader> reader,
                 unsigned char *first_block);

class Mark4_reader : public Mark4_reader_interface {
  enum Debug_level {
    NO_CHECKS = 0,
    CHECK_PERIODIC_HEADERS,
    CHECK_ALL_HEADERS,
    CHECK_BIT_STATISTICS
  };
public:

  // unsigned char buffer[SIZE_MK4_FRAME] contains the beginning of a mark4-frame
  Mark4_reader(boost::shared_ptr<Data_reader> data_reader,
               int N,
               unsigned char *buffer,
               unsigned char *mark4_block);
  virtual ~Mark4_reader();

  /// Time in microseconds
  /// Changed the order of the arguments when I changed from miliseconds to microseconds
  int64_t goto_time(unsigned char *mark4_block,
                    int64_t us_time);

  /// Get the current time in microseconds
  int64_t get_current_time();

  /// Read another mark4-frame
  bool read_new_block(unsigned char *mark4_block);

  /// Get track information from a mark4 header
  std::vector< std::vector<int> >
  get_tracks(const Input_node_parameters &input_node_param,
             unsigned char *mark4_block);


  int time_between_headers() {
    return MARK4_TRACK_BIT_RATE / SIZE_MK4_FRAME;
  }

private:
  // format a time in miliseconds
  std::string time_to_string(int64_t time);

  // checking the header:
  bool check_time_stamp(Mark4_header &header);
  bool check_track_bit_statistics(unsigned char *mark4_block);
private:
  // Data reader: input stream
  boost::shared_ptr<Data_reader> data_reader_;

  // Time information
  int start_day_;
  // start time and current time in miliseconds
  // start time is used to check the data rate
  int64_t start_time_, current_time_;

  // For testing
  Debug_level debug_level_;
  int block_count_;

public:
  const int N;
};


int find_start_of_header(boost::shared_ptr<Data_reader> reader,
                         unsigned char first_block[]);


#endif // MARK4_READER_H