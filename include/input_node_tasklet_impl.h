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

template <class Type>
class Input_node_tasklet_implementation : public Input_node_tasklet {
public:
  typedef Mark4_reader_tasklet<Type>                       Mark4_reader_tasklet_;
  typedef Integer_delay_correction_all_channels<Type>      Integer_delay_tasklet_;
  typedef Void_consuming_tasklet<typename Input_node_types<Type>::Fft_buffer >
  /**/                                                     Void_consuming_tasklet_;
  
  Input_node_tasklet_implementation(Data_reader *reader, char *buffer);

  void do_task();

  int goto_time(int time);

  bool append_time_slice(const Time_slice &time_slice);
private:
  std::list<Time_slice>               time_slices_;
  Mark4_reader_tasklet_               mark4_reader_;
  Integer_delay_tasklet_              integer_delay_;

  Void_consuming_tasklet_             void_consumer_;
};


/// Implementation

template <class Type>
Input_node_tasklet_implementation<Type>::
Input_node_tasklet_implementation(Data_reader *reader, char *buffer)
    : mark4_reader_(reader, buffer) {
  integer_delay_.connect_to(mark4_reader_.get_output_buffer());
  void_consumer_.connect_to(integer_delay_.get_output_buffer());
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
  if (void_consumer_.has_work()) {
    void_consumer_.do_task();
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
