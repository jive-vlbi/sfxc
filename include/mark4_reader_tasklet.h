/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef MARK4_READER_TASKLET_H
#define MARK4_READER_TASKLET_H

#include <boost/shared_ptr.hpp>

#include "tasklet/tasklet.h"
#include "mark4_reader.h"
#include "input_node_types.h"

template <class Type>
class Mark4_reader_tasklet : public Tasklet {
public:
  typedef typename Input_node_types<Type>::Mk4_memory_pool    Input_memory_pool;
  typedef typename Input_node_types<Type>::Mk4_buffer_element Input_element;
  typedef typename Input_node_types<Type>::Mk4_buffer         Output_buffer;
  typedef typename Input_node_types<Type>::Mk4_buffer_element Output_buffer_element;
  typedef typename Input_node_types<Type>::Mk4_buffer_ptr     Output_buffer_ptr;

  Mark4_reader_tasklet(boost::shared_ptr<Data_reader> reader,
                       char *buffer);

  /// For Tasklet
  void do_task();
  /// For Tasklet
  bool has_work();
  const char *name() {
    return "Mark4_reader_tasklet";
  }

  /// Get the output
  Output_buffer_ptr get_output_buffer();

  /// Goto a time in the future.
  int goto_time(int time);

  /// get the current time in miliseconds
  int get_current_time();

  /// set a stop time (after which no data is sent)
  void set_stop_time(int64_t time);

  std::vector< std::vector<int> > get_tracks(const Input_node_parameters &input_node_param);

private:
  /// Get an element from the memory pool into input_element_
  void allocate_element();
  /// Push the input_element_ to the output buffer
  void push_element();

private:
  /// Data stream to read from
  boost::shared_ptr< Mark4_reader<Type> > mark4_reader_;
  /// Memory pool of data block that can be filled
  Input_memory_pool                   memory_pool_;
  /// Current mark4 data block
  Input_element                       input_element_;
  /// Output buffer of mark4 data blocks
  Output_buffer_ptr                   output_buffer_;

  /// Stop time in microseconds
  int64_t stop_time;
};

template <class Type>
Mark4_reader_tasklet<Type>::
Mark4_reader_tasklet(boost::shared_ptr<Data_reader> reader, char *buffer)
    : memory_pool_(10), stop_time(-1) {
  output_buffer_ = Output_buffer_ptr(new Output_buffer());
  allocate_element();
  mark4_reader_ =
    boost::shared_ptr<Mark4_reader<Type> >(new Mark4_reader<Type>(reader,
                                           buffer,
                                           &input_element_.data()[0]));

}

template <class Type>
void
Mark4_reader_tasklet<Type>::
do_task() {
  assert(has_work());

  push_element();
  allocate_element();
  mark4_reader_->read_new_block(&input_element_.data()[0]);
}

template <class Type>
bool
Mark4_reader_tasklet<Type>::
has_work() {
  if (memory_pool_.empty())
    return false;
  // NGHK: TODO check
//  DEBUG_MSG("time_stamp: " << mark4_reader_->get_current_time() << " < " <<  stop_time);
  if ((stop_time > 0) && (stop_time <= mark4_reader_->get_current_time()))
    return false;

  return true;
}

template <class Type>
void
Mark4_reader_tasklet<Type>::
allocate_element() {
  assert(!memory_pool_.empty());
  input_element_ = memory_pool_.allocate();
  std::vector<Type> vector_ = input_element_.data();
  if (vector_.size() != SIZE_MK4_FRAME) {
    vector_.resize(SIZE_MK4_FRAME);
    input_element_.data().resize(SIZE_MK4_FRAME);
  }
}

template <class Type>
int
Mark4_reader_tasklet<Type>::
goto_time(int ms_time) {
  int64_t us_time = int64_t(1000)*ms_time;
  // NGHK: TODO: check if we need to release the current block
  int64_t new_time =
    mark4_reader_->goto_time(&input_element_.data()[0], us_time);
  
  if (us_time != new_time) {
    DEBUG_MSG("New time " << us_time << "us not found. Current time is " << new_time);
  }
  return new_time/1000;
}

template <class Type>
int
Mark4_reader_tasklet<Type>::
get_current_time() {
  return mark4_reader_->get_current_time();
}

template <class Type>
void
Mark4_reader_tasklet<Type>::
set_stop_time(int64_t time) {
  assert(mark4_reader_->get_current_time() < time);
  stop_time = time;
}



template <class Type>
void
Mark4_reader_tasklet<Type>::
push_element() {
//  Mark4_header<Type>        header;
//  header.set_header(&input_element_.data()[0]);
//  DEBUG_MSG(header.get_time_str(0));

  output_buffer_->push(input_element_);
}

template <class Type>
typename Mark4_reader_tasklet<Type>::Output_buffer_ptr
Mark4_reader_tasklet<Type>::
get_output_buffer() {
  return output_buffer_;
}

template <class Type>
std::vector< std::vector<int> >
Mark4_reader_tasklet<Type>::
get_tracks(const Input_node_parameters &input_node_param) {
  return mark4_reader_->get_tracks(input_node_param,
                                   &input_element_.data()[0]);
}
#endif // MARK4_READER_TASKLET_H