/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef INPUT_DATA_FORMAT_READER_H
#define INPUT_DATA_FORMAT_READER_H

#include "data_reader.h"
#include "control_parameters.h"
#include "input_node_types.h"

#include <boost/shared_ptr.hpp>

class Input_data_format_reader {
public:
  typedef Input_node_types::Input_data_frame            Data_frame;
  typedef Input_node_types::value_type                  value_type;
  Input_data_format_reader(boost::shared_ptr<Data_reader> data_reader);
  virtual ~Input_data_format_reader();

  /// Time in microseconds
  virtual int64_t goto_time(Data_frame &data, int64_t us_time) = 0;

  /// Get the current time in microseconds
  virtual int64_t get_current_time() = 0;

  /// Read another mark5a-frame
  virtual bool read_new_block(Data_frame &data) = 0;

  /// Get track information from a mark5a header
  virtual std::vector< std::vector<int> >
  get_tracks(const Input_node_parameters &input_node_param, 
             Data_frame &data)= 0;

  virtual size_t bytes_per_input_word() const = 0;
  virtual size_t size_data_block() const = 0;

  virtual int time_between_headers() = 0;

  bool eof();

  virtual void set_parameters(const Input_node_parameters &param) = 0;

  virtual TRANSPORT_TYPE get_transport_type() const = 0;

protected:
  // Data reader: input stream
  boost::shared_ptr<Data_reader> data_reader_;

//   int data_rate() const;
};

#endif // INPUT_DATA_FORMAT_READER_H
