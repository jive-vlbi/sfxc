/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef MARK5b_READER_TASKLET_H
#define MARK5b_READER_TASKLET_H

#include <boost/shared_ptr.hpp>

#include "tasklet/tasklet.h"
#include "mark5b_reader.h"
#include "input_node_types.h"

#ifdef RUNTIME_STATISTIC
#include "monitor.h"
#endif // RUNTIME_STATISTIC

class Mark5b_reader_tasklet : public Tasklet {
public:
  typedef boost::shared_ptr< Mark5b_reader >    Mark5b_reader_ptr;
  typedef Input_node_types::value_type         value_type;
  typedef Input_node_types::Mark5_memory_pool    Input_memory_pool;
  typedef Input_node_types::Mark5_buffer_element Input_element;
  typedef Input_node_types::Mark5_buffer         Output_buffer;
  typedef Input_node_types::Mark5_buffer_element Output_buffer_element;
  typedef Input_node_types::Mark5_buffer_ptr     Output_buffer_ptr;

  Mark5b_reader_tasklet(Mark5b_reader_ptr mark5b_reader);

  /// For Tasklet
  void do_task();

  /// For Tasklet
  bool has_work();

  const char* name() {
    return "Mark5b_reader_tasklet";
  }

  /// Get the output
  Output_buffer_ptr get_output_buffer();

  /// Goto a time in the future.
  int goto_time(int time);

  /// get the current time in miliseconds
  int get_current_time();
  /// get the stop time in miliseconds
  int get_stop_time();

  /// set a stop time (after which no data is sent)
  void set_stop_time(int64_t time);

  /// set parameters (the track bit rate)
  void set_parameters(const Input_node_parameters &input_param);

  std::vector< std::vector<int> > get_tracks(const Input_node_parameters &input_node_param);

  int size_input_word() const {
    // Mark5b has a fixed input size
    return sizeof(int32_t);
  }

private:
  /// Get an element from the memory pool into input_element_
  void allocate_element();
  /// Push the input_element_ to the output buffer
  void push_element();

private:
  /// Data stream to read from
  Mark5b_reader_ptr                    mark5b_reader_;
  /// Memory pool of data block that can be filled
  Input_memory_pool                   memory_pool_;
  /// Current mark5b data block
  Input_element                       input_element_;
  /// Output buffer of mark5b data blocks
  Output_buffer_ptr                   output_buffer_;

  /// Current time in microseconds
  int64_t current_time;

  /// Stop time in microseconds
  int64_t stop_time;

#ifdef RUNTIME_STATISTIC
  QOS_MonitorSpeed monitor_;
#endif // RUNTIME_STATISTIC
};

#endif // MARK5B_READER_TASKLET_H

