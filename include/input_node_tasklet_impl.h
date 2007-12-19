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
#include "void_consuming_tasklet.h"
#include "channel_extractor.h"

template <class Type>
class Input_node_tasklet_implementation : public Input_node_tasklet {
public:
  typedef Mark4_reader_tasklet<Type>                       Mark4_reader_tasklet_;
  typedef Integer_delay_correction_all_channels<Type>      Integer_delay_tasklet_;
  typedef Channel_extractor<Type>                          Channel_extractor_tasklet_;
  typedef Void_consuming_tasklet<typename Input_node_types<Type>::Channel_buffer>
  /**/                                                     Void_consuming_tasklet_;

  Input_node_tasklet_implementation(Data_reader *reader, char *buffer);

  void do_task();

  // Inherited from Input_node_tasklet
  void set_delay_table(Delay_table_akima &delay);
  void set_parameters(Track_parameters &track_param);
  int goto_time(int time);
  bool append_time_slice(const Time_slice &time_slice);

private:
  std::list<Time_slice>                time_slices_;
  Mark4_reader_tasklet_                mark4_reader_;
  Channel_extractor_tasklet_           channel_extractor_;
  Integer_delay_tasklet_               integer_delay_;

  std::vector<Void_consuming_tasklet_> void_consumers_;
};


/// Implementation

template <class Type>
Input_node_tasklet_implementation<Type>::
Input_node_tasklet_implementation(Data_reader *reader, char *buffer)
    : mark4_reader_(reader, buffer) {
  integer_delay_.connect_to(mark4_reader_.get_output_buffer());
  channel_extractor_.connect_to(integer_delay_.get_output_buffer());
}


template <class Type>
void
Input_node_tasklet_implementation<Type>::
do_task() {
  if (mark4_reader_.has_work()) {
    mark4_reader_.do_task();
  }
  if (integer_delay_.has_work()) {
    integer_delay_.do_task();
  }
  if (channel_extractor_.has_work()) {
    channel_extractor_.do_task();
  }
  for (size_t i=0; i<void_consumers_.size(); i++) {
    if (void_consumers_[i].has_work()) {
      void_consumers_[i].do_task();
    }
  }
}

template <class Type>
void
Input_node_tasklet_implementation<Type>::
set_delay_table(Delay_table_akima &delay) {
  assert(false);
}

template <class Type>
void
Input_node_tasklet_implementation<Type>::
set_parameters(Track_parameters &track_param) {
  //  integer_delay_.set_parameters(track_param);
  channel_extractor_.set_parameters(track_param,
                                    mark4_reader_.get_tracks(track_param));

  size_t number_frequency_channels = track_param.channels.size();
  void_consumers_.resize(number_frequency_channels);
  for (size_t i=0; i < number_frequency_channels; i++) {
    void_consumers_[i].connect_to(channel_extractor_.get_output_buffer(i));
  }
}

template <class Type>
int
Input_node_tasklet_implementation<Type>::
goto_time(int time) {
  return mark4_reader_.goto_time(time);
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

#endif // INPUT_NODE_TASKLET_IMPL_H
