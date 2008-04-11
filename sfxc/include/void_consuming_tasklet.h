/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef VOID_CONSUMING_TASKLET_H
#define VOID_CONSUMING_TASKLET_H

#include "tasklet/tasklet.h"
#include "utils.h"

template <class Buffer>
class Void_consuming_tasklet : public Tasklet {
public:
  typedef Buffer                            Input_buffer;
  typedef typename Input_buffer::value_type Input_buffer_element;
  typedef boost::shared_ptr<Input_buffer>   Input_buffer_ptr;

  Void_consuming_tasklet();

  /// For tasklet
  void do_task();
  bool has_work();
  void connect_to(Input_buffer_ptr buffer);
  const char *name() {
    return "Void_consuming_tasklet";
  }

private:
  Input_buffer_ptr input_buffer_;
};

template <class Buffer>
Void_consuming_tasklet<Buffer>::Void_consuming_tasklet() {
}

template <class Buffer>
void
Void_consuming_tasklet<Buffer>::do_task() {
  assert(has_work());
#if 0
  int i;
  input_buffer_->consume(i);
  //DEBUG_MSG("popped " << i);
  input_buffer_->consumed();
#else
  input_buffer_->front().release();
  input_buffer_->pop();
#endif
}

template <class Buffer>
bool
Void_consuming_tasklet<Buffer>::has_work() {
  if (input_buffer_ == Input_buffer_ptr())
    return false;
  return !input_buffer_->empty();
}

template <class Buffer>
void
Void_consuming_tasklet<Buffer>::
connect_to(Input_buffer_ptr buffer) {
  input_buffer_ = buffer;
}

#endif // VOID_CONSUMING_TASKLET_H
