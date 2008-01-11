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

#include "input_node_tasklet.h"
#include "input_node_types.h"
#include "utils.h"

#include "mark4_reader_tasklet.h"
#include "integer_delay_correction_all_channels.h"
#include "eavesdropping_tasklet.h"
#include "void_consuming_tasklet.h"
#include "channel_extractor.h"
#include "input_node_data_writer_tasklet.h"

template <class Type>
class Input_node_tasklet_implementation : public Input_node_tasklet {
public:
  typedef Mark4_reader_tasklet<Type>                       Mark4_reader_tasklet_;
  typedef Integer_delay_correction_all_channels<Type>      Integer_delay_tasklet_;
  typedef Channel_extractor<Type>                          Channel_extractor_tasklet_;
  typedef Input_node_data_writer_tasklet<Type>             Data_writer_tasklet_;

  Input_node_tasklet_implementation(Data_reader *reader, char *buffer);

  void do_task();
  bool has_work();

  const char *name() {
    return __PRETTY_FUNCTION__;
  }


  // Inherited from Input_node_tasklet
  void set_delay_table(Delay_table_akima &delay);
  void set_parameters(Input_node_parameters &input_node_param);
  int goto_time(int time);
  void set_stop_time(int time);
  bool append_time_slice(const Time_slice &time_slice);
  void add_data_writer(size_t i,
                       Data_writer_ptr_ data_writer,
                       int nr_integrations);

private:
  std::list<Time_slice>                time_slices_;
  Mark4_reader_tasklet_                mark4_reader_;
  Integer_delay_tasklet_               integer_delay_;
  Channel_extractor_tasklet_           channel_extractor_;

  std::vector<Data_writer_tasklet_>    data_writers_;

  bool did_work;
};


/// Implementation

template <class Type>
Input_node_tasklet_implementation<Type>::
Input_node_tasklet_implementation(Data_reader *reader, char *buffer)
    : mark4_reader_(reader, buffer), did_work(true) {
  integer_delay_.connect_to(mark4_reader_.get_output_buffer());
  channel_extractor_.connect_to(integer_delay_.get_output_buffer());
}


template <class Type>
void
Input_node_tasklet_implementation<Type>::
do_task() {
  did_work = false;
  if (mark4_reader_.has_work()) {
    //DEBUG_MSG(mark4_reader_.name());
    mark4_reader_.do_task();
    did_work = true;
  }
  if (integer_delay_.has_work()) {
    //DEBUG_MSG(integer_delay_.name());
    integer_delay_.do_task();
    did_work = true;
  }
  if (channel_extractor_.has_work()) {
    //DEBUG_MSG(channel_extractor_.name());
    channel_extractor_.do_task();
    did_work = true;
  }
  for (size_t i=0; i<data_writers_.size(); i++) {
    if (data_writers_[i].has_work()) {
      //DEBUG_MSG(i << " " << void_consumers_[i].name());
      data_writers_[i].do_task();
      did_work = true;
    }
  }
  //DEBUG_MSG("Input_node_tasklet_implementation::do_task() finished");
}

template <class Type>
bool
Input_node_tasklet_implementation<Type>::
has_work() {
  return did_work;
}

template <class Type>
void
Input_node_tasklet_implementation<Type>::
set_delay_table(Delay_table_akima &table) {
  integer_delay_.set_delay_table(table);
}

template <class Type>
void
Input_node_tasklet_implementation<Type>::
set_parameters(Input_node_parameters &input_node_param) {
  integer_delay_.set_parameters(input_node_param);

  channel_extractor_.set_parameters(input_node_param,
                                    mark4_reader_.get_tracks(input_node_param));

  size_t number_frequency_channels = input_node_param.channels.size();
  data_writers_.resize(number_frequency_channels);

  for (size_t i=0; i < number_frequency_channels; i++) {
    data_writers_[i].connect_to(channel_extractor_.get_output_buffer(i));
    data_writers_[i].set_parameters(input_node_param);
  }
}

template <class Type>
int
Input_node_tasklet_implementation<Type>::
goto_time(int time) {
  int new_time = mark4_reader_.goto_time(time);
  integer_delay_.set_time(new_time);
  return new_time;
}
template <class Type>
void
Input_node_tasklet_implementation<Type>::
set_stop_time(int time) {
  return mark4_reader_.set_stop_time(time);
}

template <class Type>
bool
Input_node_tasklet_implementation<Type>::
append_time_slice(const Time_slice &time_slice) {
  if (!time_slices_.empty()) {
    if (time_slices_.end()->stop_time > time_slice.start_time) {
      DEBUG_MSG("Time slice starts before the end of the last time slice.");
      DEBUG_MSG("Not adding the time slice.");
      return false;
    }
  }

  time_slices_.push_back(time_slice);

  return true;
}

template <class Type>
void
Input_node_tasklet_implementation<Type>::
add_data_writer(size_t i,
                Data_writer_ptr_ data_writer,
                int nr_integrations) {
  assert(i < data_writers_.size());
  data_writers_[i].add_data_writer(data_writer, nr_integrations);
}

#endif // INPUT_NODE_TASKLET_IMPL_H
