/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2008
 *            Aard Keimpema <keimpema@jive.nl>, 2009
 *
 *  This file contains:
 *     - The declaration of the Input_node_data_writer object. This is object
 *       is used to stream data to the correlation nodes. The object is not
 *       thread safe an thus can only use one client.
 */
#ifndef INPUT_NODE_DATA_WRITER_H_INCLUDED
#define INPUT_NODE_DATA_WRITER_H_INCLUDED

#include <boost/shared_ptr.hpp>
#include "data_writer.h"
#include "utils.h"
#include "thread.h"
#include "timer.h"
#include "rttimer.h"
#include "input_node_types.h"
#include "control_parameters.h"

/// Forward declaration
class Input_node_data_writer;

/// A smart pointer to this object
typedef boost::shared_ptr<Input_node_data_writer> Input_node_data_writer_sptr;

class Input_node_data_writer : public Thread 
{
public:
  typedef Input_node_types::Channel_buffer          Input_buffer;
  typedef Input_node_types::Channel_buffer_element  Input_buffer_element;
  typedef boost::shared_ptr<Input_buffer>           Input_buffer_ptr;

  // The writer can, in principle, occur multiple times in the writer_queue
  // so we store the slice size in a separate integer and set
  // set_size_dataslice(slice_size) when we start writing
  struct Writer_struct {
    Writer_struct():active(false) {}
    Data_writer_sptr writer;
    int64_t         slice_size;
    bool            active;
  };
  typedef Threadsafe_queue< Writer_struct >      Data_writer_queue;
  Input_node_data_writer();
  virtual ~Input_node_data_writer();

  void do_execute();

  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);

  void add_timeslice(Data_writer_sptr data_writer, int64_t nr_samples);

	/// return the amount of data sent...
  uint64_t do_task();


  /// The queue storing all the intervals
  Threadsafe_queue<Time_interval> intervals_;

  bool has_work();
  const char *name() {
    return __PRETTY_FUNCTION__;
  }

  void set_parameters(const Input_node_parameters &input_param);

  /// Empty the input queue, called from the destructor of Input_node
  void empty_input_queue();

  /// Allocate a new shared pointer to this object.
  static Input_node_data_writer_sptr new_sptr();

  ///  Add a new delay table to the writer, only contains the delay at time 
  /// positions where the integer delay changes
  void add_delay(Delay_memory_pool_element delay);

  void add_time_interval(Time &start, Time &stop);
  void fetch_next_time_interval();
  Time get_current_time();

private:
  Input_buffer_ptr    input_buffer_;
  Data_writer_queue   data_writers_;
  int                 delay_index;
  int sample_rate;
  int bits_per_sample;
  Time integration_time;
  bool sync_stream;

  /// The queue storing all the delays
  Threadsafe_queue<Delay_memory_pool_element> delays_;
  Delay_memory_pool_element delay_list;

  int get_next_delay_pos(std::vector<Delay> &cur_delay, Time start_time);

  void write_invalid(Data_writer_sptr writer, int nInvalid);
  void write_delay(Data_writer_sptr writer, int8_t delay);
  void write_data(Data_writer_sptr writer, int ndata, int byte_offset);
  void write_invalid_blocks(Data_writer_sptr writer, int byte_offset, int n_bytes, 
                            int invalid_samples_per_block, int block_size);
  void write_end_of_stream(Data_writer_sptr writer);

  int64_t write_initial_invalid_data(Writer_struct &data_writer, int64_t byte_offset);
  uint64_t total_data_written_;
  int block_size;

  uint64_t data_written_in_slice_;
  uint64_t size_of_slice_;

  /// The currently processed interval
  Time_interval current_interval_;

  Time _current_time;
  Time _slice_start;
  Time byte_length;

  double last_duration_;
  RTTimer timer_waiting_;
  RTTimer timer_other_;
  RTTimer timer_writing_;
  int interval;
};

inline Time
Input_node_data_writer::get_current_time(){
  return _current_time;
}
#endif // INPUT_NODE_DATA_WRITER_H_INCLUDED
