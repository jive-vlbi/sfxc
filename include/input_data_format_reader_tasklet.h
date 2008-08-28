/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef INPUT_DATA_FORMAT_READER_TASKLET_H
#define INPUT_DATA_FORMAT_READER_TASKLET_H

#include <boost/shared_ptr.hpp>

#include "tasklet/tasklet.h"
#include "thread.h"

#include "mark5a_reader.h"
#include "input_node_types.h"

#include "timer.h"

#ifdef RUNTIME_STATISTIC
#include "monitor.h"
#endif // RUNTIME_STATISTIC

class Input_data_format_reader_tasklet : public Tasklet, public Thread {
public:
  typedef boost::shared_ptr< Input_data_format_reader > Data_format_reader_ptr;
  typedef Input_data_format_reader::Data_frame          Data_frame;
  typedef Input_node_types::value_type                  value_type;
  typedef Input_node_types::Mark5_memory_pool           Input_memory_pool;
  typedef Input_node_types::Mark5_buffer_element        Input_element;
  typedef Input_node_types::Mark5_buffer                Output_buffer;
  typedef Input_node_types::Mark5_buffer_element        Output_buffer_element;
  typedef Input_node_types::Mark5_buffer_ptr            Output_buffer_ptr;

  Input_data_format_reader_tasklet(Data_format_reader_ptr reader,
                                   Data_frame &data);

  ~Input_data_format_reader_tasklet();

  /// The main thread.
  void do_execute();
  void stop();
  void fetch_next_time_interval();

  /// set a time interval to process (after which the tasklet is blocking)
  /// The start and stop time are given in micro-seconds.
  void add_time_interval(uint64_t us_start_time, uint64_t us_stop_time);

  /// For Tasklet
  void do_task();

  /// For Tasklet
  bool has_work();

  const char* name() {
    return "Input_data_format_reader_tasklet";
  }

  /// Get the output
  Output_buffer_ptr get_output_buffer();

  /// Goto a time in the future.
  /// Given in micro-second
  uint64_t goto_time(uint64_t time);

  /// get the current time in miliseconds
  uint64_t get_current_time();

  /// get the stop time in miliseconds
  int get_stop_time();

  /// set a stop time (after which no data is sent)
  void set_stop_time(int64_t time);

  std::vector< std::vector<int> > get_tracks(const Input_node_parameters &input_node_param);

  int size_input_word() const {
    return reader_->bytes_per_input_word();
  }

  void set_parameters(const Input_node_parameters &input_node_param) {
    reader_->set_parameters(input_node_param);
  }

	inline uint64_t get_num_processed_bytes(){ return data_read_; }

private:
  /// Get an element from the memory pool into input_element_
  void allocate_element();
  /// Push the input_element_ to the output buffer
  void push_element();
  /// Randomize data in the mark5a block
  void randomize_block();

private:
  /// Data stream to read from
  Data_format_reader_ptr              reader_;
  /// Memory pool of data block that can be filled
  Input_memory_pool                   memory_pool_;
  /// Current mark5a data block
  Input_element                       input_element_;
  /// Output buffer of mark5a data blocks
  Output_buffer_ptr                   output_buffer_;


  /// The current interval to process
  Time_interval current_interval_;


  /// Storing all the pending interval to process
  Threadsafe_queue<Time_interval>     intervals_;



  /// Current time in microseconds
  int64_t current_time;

  /// Stop time in microseconds
  int64_t stop_time;

	/// Amount of data that was received by this component
  uint64_t data_read_;

  const size_t n_bytes_per_input_word;
};

#endif // INPUT_DATA_FORMAT_READER_TASKLET_H

