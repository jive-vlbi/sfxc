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
#include "correlator_time.h"

#include <boost/shared_ptr.hpp>

#define RESYNC_MAX_DATA_FRAMES  16

class Input_data_format_reader {
public:
  typedef Input_node_types::Input_data_frame            Data_frame;
  typedef Input_node_types::value_type                  value_type;
  Input_data_format_reader(boost::shared_ptr<Data_reader> data_reader);
  virtual ~Input_data_format_reader();

  virtual bool open_input_stream(Data_frame &data) = 0;
  virtual Time goto_time(Data_frame &data, Time time) = 0;

  /// Get the current time
  virtual Time get_current_time() = 0;

  /// Read another mark5a-frame
  virtual bool read_new_block(Data_frame &data) = 0;

  virtual size_t bytes_per_input_word() const = 0;
  virtual size_t size_data_block() const = 0;

  virtual Time time_between_headers() = 0;

  bool eof();
  void find_fill_pattern(Data_frame &data);
  bool is_open(){
    return is_open_;
  }

  virtual void set_parameters(const Input_node_parameters &param) = 0;

  virtual TRANSPORT_TYPE get_transport_type() const = 0;

protected:
  // Data reader: input stream
  boost::shared_ptr<Data_reader> data_reader_;
  // Set to true if there is a valid header found in the data stream
  bool is_open_;
};

#endif // INPUT_DATA_FORMAT_READER_H
