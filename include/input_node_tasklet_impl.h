/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef INPUT_NODE_TASKLET_IMPL_H
#define INPUT_NODE_TASKLET_IMPL_H

#include <memory_pool.h>
#include "timer.h"

#include "input_node_tasklet.h"
#include "input_node_types.h"
#include "utils.h"

#include "mark4_reader_tasklet.h"
#include "integer_delay_correction_per_channel.h"
#include "void_consuming_tasklet.h"
#include "channel_extractor_tasklet.h"
#include "input_node_data_writer_tasklet.h"

#include "monitor.h"

class Input_node_tasklet_implementation : public Input_node_tasklet {
public:
  typedef Mark4_reader_tasklet                       Mark4_reader_tasklet_;
  typedef Channel_extractor_tasklet                  Channel_extractor_tasklet_;
  typedef Integer_delay_correction_per_channel       Integer_delay_tasklet_;
  typedef Input_node_data_writer_tasklet             Data_writer_tasklet_;

  Input_node_tasklet_implementation(boost::shared_ptr<Data_reader> reader,
                                    char *buffer,
                                    size_t n_bytes_per_input_word_);

  ~Input_node_tasklet_implementation() {

#if PRINT_TIMER
    DEBUG_MSG("Time mar4_reader:       " << mark4_reader_timer_.measured_time());
    DEBUG_MSG("Time integer_delay:     " << integer_delay_timer_.measured_time());
    DEBUG_MSG("Time channel_extractor: " << channel_extractor_timer_.measured_time());
    DEBUG_MSG("Time data_writers:      " << data_writers_timer_.measured_time());
#endif

  }

  void do_task();
  bool has_work();

  const char *name() {
    return __PRETTY_FUNCTION__;
  }


  // Inherited from Input_node_tasklet
  void set_delay_table(Delay_table_akima &delay);
  void set_parameters(const Input_node_parameters &input_node_param,
                      int node_nr);
  int goto_time(int time);
  int get_current_time();
  void set_stop_time(int time);
  //  bool append_time_slice(const Time_slice &time_slice);
  void add_data_writer(size_t i,
                       Data_writer_ptr_ data_writer,
                       int nr_seconds);

private:
  //  std::list<Time_slice>                time_slices_;
  Mark4_reader_tasklet_                mark4_reader_;
  Channel_extractor_tasklet_           channel_extractor_;

  // Pointer because we can not copy construct the Integer_delay_tasklet_
  // because of the memory pool
  std::vector<Integer_delay_tasklet_ *>  integer_delay_;
  std::vector<Data_writer_tasklet_>    data_writers_;

  bool did_work;

  Timer mark4_reader_timer_, integer_delay_timer_, channel_extractor_timer_, data_writers_timer_;

  Delay_table_akima delay_table;

#ifdef RUNTIME_STATISTIC
  QOS_MonitorSpeed mark4reader_state_;
  QOS_MonitorSpeed chex_state_;
  QOS_MonitorSpeed integerdelay_state_;
  QOS_MonitorSpeed outputwriter_state_;
  QOS_MonitorSpeed dotask_state_;
#endif //RUNTIME_STATISTIC
  const size_t n_bytes_per_input_word;
};


#endif // INPUT_NODE_TASKLET_IMPL_H
