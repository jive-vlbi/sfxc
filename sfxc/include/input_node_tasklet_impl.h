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
#include "integer_delay_correction_all_channels.h"
#include "void_consuming_tasklet.h"
#include "channel_extractor_tasklet.h"
#include "input_node_data_writer_tasklet.h"

template <class Type>
class Input_node_tasklet_implementation : public Input_node_tasklet {
public:
  typedef Mark4_reader_tasklet<Type>                       Mark4_reader_tasklet_;
  typedef Integer_delay_correction_all_channels<Type>      Integer_delay_tasklet_;
  typedef Channel_extractor_tasklet<Type>                  Channel_extractor_tasklet_;
  typedef Input_node_data_writer_tasklet<Type>             Data_writer_tasklet_;

  Input_node_tasklet_implementation(boost::shared_ptr<Data_reader> reader, char *buffer);

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
  Integer_delay_tasklet_               integer_delay_;
  Channel_extractor_tasklet_           channel_extractor_;

  std::vector<Data_writer_tasklet_>    data_writers_;

  bool did_work;

  Timer mark4_reader_timer_, integer_delay_timer_, channel_extractor_timer_, data_writers_timer_;

};


/// Implementation

template <class Type>
Input_node_tasklet_implementation<Type>::
Input_node_tasklet_implementation(boost::shared_ptr<Data_reader> reader, char *buffer)
    : mark4_reader_(reader, buffer),
did_work(true) {
  integer_delay_.connect_to(mark4_reader_.get_output_buffer());
  channel_extractor_.connect_to(integer_delay_.get_output_buffer());
}


template <class Type>
void
Input_node_tasklet_implementation<Type>::
do_task() {
  did_work = false;
  if (integer_delay_.time_set()) {
    mark4_reader_timer_.resume();
    if (mark4_reader_.has_work()) {
      mark4_reader_.do_task();
      did_work = true;
    }
    mark4_reader_timer_.stop();
    integer_delay_timer_.resume();
    if (integer_delay_.has_work()) {
      integer_delay_.do_task();
      did_work = true;
    }
    integer_delay_timer_.stop();
    channel_extractor_timer_.resume();
    if (channel_extractor_.has_work()) {
      channel_extractor_.do_task();
      did_work = true;
    }
    channel_extractor_timer_.stop();
    data_writers_timer_.resume();
    for (size_t i=0; i<data_writers_.size(); i++) {
      if (data_writers_[i].has_work()) {
        data_writers_[i].do_task();
        did_work = true;
      }
    }
    data_writers_timer_.stop();
  }
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

  did_work = true;
}

template <class Type>
void
Input_node_tasklet_implementation<Type>::
set_parameters(const Input_node_parameters &input_node_param,
               int node_nr) {
  integer_delay_.set_parameters(input_node_param, node_nr);

  channel_extractor_.set_parameters(input_node_param,
                                    mark4_reader_.get_tracks(input_node_param));

  size_t number_frequency_channels = input_node_param.channels.size();
  data_writers_.resize(number_frequency_channels);

  for (size_t i=0; i < number_frequency_channels; i++) {
    data_writers_[i].connect_to(channel_extractor_.get_output_buffer(i));
    data_writers_[i].set_parameters(input_node_param);
  }

  did_work = true;
}

template <class Type>
int
Input_node_tasklet_implementation<Type>::
goto_time(int time) {
  int new_time = mark4_reader_.goto_time(time);
  integer_delay_.set_time(int64_t(1000)*new_time);

  did_work = true;
  return new_time;
}
template <class Type>
int
Input_node_tasklet_implementation<Type>::
get_current_time() {
  return mark4_reader_.get_current_time();
}
template <class Type>
void
Input_node_tasklet_implementation<Type>::
set_stop_time(int time) {
  did_work = true;

  integer_delay_.set_stop_time(int64_t(1000)*time);
  return mark4_reader_.set_stop_time(int64_t(1000)*time);
}

template <class Type>
void
Input_node_tasklet_implementation<Type>::
add_data_writer(size_t i,
                Data_writer_ptr_ data_writer,
                int nr_seconds) {
  did_work = true;
  assert(i < data_writers_.size());
  int size_slice = integer_delay_.bytes_of_output(nr_seconds);
  data_writers_[i].add_data_writer(data_writer, size_slice);
}

#endif // INPUT_NODE_TASKLET_IMPL_H
