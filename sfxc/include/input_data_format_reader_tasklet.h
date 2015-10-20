/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
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
#include "correlator_time.h"
#ifdef RUNTIME_STATISTIC
#include "monitor.h"
#endif // RUNTIME_STATISTIC

class Input_data_format_reader_tasklet : public Tasklet, public Thread {
public:
  typedef boost::shared_ptr< Input_data_format_reader > Data_format_reader_ptr;
  typedef Input_data_format_reader::Data_frame          Data_frame;
  typedef Input_node_types::value_type                  value_type;
  typedef Input_node_types::Data_memory_pool            Input_memory_poolzor;
  typedef boost::shared_ptr<Input_memory_poolzor>          Input_memory_pool_ptr;
  typedef Input_node_types::Input_data_frame            Input_element;
  typedef Input_node_types::Input_buffer                Output_buffer;
//  typedef Input_node_types::Input_buffer_element        Output_buffer_element;
  typedef Input_node_types::Input_buffer_ptr            Output_buffer_ptr;

  Input_data_format_reader_tasklet(Data_format_reader_ptr reader,
                                   Input_memory_pool_ptr memory_pool);

  ~Input_data_format_reader_tasklet();

  /// The main thread.
  void do_execute();
  void stop();
  void fetch_next_time_interval();

  /// set a time interval to process (after which the tasklet is blocking)
  /// The start and stop time are given in micro-seconds.
  void add_time_interval(Time &us_start_time, Time &us_stop_time);

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
  Time goto_time(Time time);

  /// get the current time in miliseconds
  Time get_current_time();

  int size_input_word() const {
    return reader_->bytes_per_input_word();
  }

  Data_format_reader_ptr get_data_reader(){
    return reader_;
  }

  void set_parameters(const Input_node_parameters &input_node_param);

  inline uint64_t get_num_processed_bytes(){ return data_read_; }

private:
  /// Get an element from the memory pool into input_element_
  void allocate_element();
  /// Push the input_element_ to the output buffer
  void push_element();
  /// Randomize data in the mark5a block
  void randomize_block();
  /// Used for data modulation (see p.6 of Mark4 memo 230A, Whitney 2005)
  void demodulate(Input_element &data);
  void gen_demodulation_sequence(int sequence_length);
  std::vector<unsigned char> demodulation_sequence; // contains the pseudo-random sequence
  void push_random_blocks(int nblocks, int channel);

private:
  /// Data stream to read from
  Data_format_reader_ptr              reader_;
  /// Memory pool of data block that can be filled
  Input_memory_pool_ptr               memory_pool_;
  /// Current mark5a data block
  Input_element                       input_element_;
  /// Start of previous mark5 block
  Time start_previous_frame;
  /// Output buffer of mark5a data blocks
  Output_buffer_ptr                   output_buffer_;

  /// The current interval to process
  Time_interval current_interval_;

  /// Storing all the pending interval to process
  Threadsafe_queue<Time_interval>     intervals_;

  /// Current time
  std::vector<Time> current_time;
  std::vector<int> nframes_left;
  Time max_time;

  // Amount of data that was received by this component
  uint64_t data_read_;

  /// Determines if data modulation is turned on(p.6 of Mark4 memo 230A, Whitney 2005)
  bool data_modulation;

  /// Determines if the correlation is aborted when the data stream contains no data
  bool exit_on_empty_datastream;

  int seqno;

  std::vector< std::vector<int> > duplicate;
};

#endif // INPUT_DATA_FORMAT_READER_TASKLET_H

