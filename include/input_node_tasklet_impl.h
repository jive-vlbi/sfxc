#ifndef INPUT_NODE_TASKLET_IMPL_H
#define INPUT_NODE_TASKLET_IMPL_H

#include "input_node_tasklet.h"
#include "mark4_reader.h"
#include "utils.h"

#include "memory_pool.h"

template <class Type>
class Input_node_tasklet_implementation : public Input_node_tasklet {
public:
  typedef Memory_pool<Type>                             Input_memory_pool;
  
  Input_node_tasklet_implementation(Mark4_reader<Type> *mark4_reader);
  
  void do_task();
  
  int goto_time(int time);
  
  bool append_time_slice(const Time_slice &time_slice);
private:
  Mark4_reader<Type>   *mark4_reader;
  
  
  std::list<Time_slice> time_slices;
  
  Memory_pool<Type>      *memory_pool;
};


/// Implementation

template <class Type>
Input_node_tasklet_implementation<Type>::
Input_node_tasklet_implementation(Mark4_reader<Type> *mark4_reader)
: mark4_reader(mark4_reader)
{
}


template <class Type>
void
Input_node_tasklet_implementation<Type>::
do_task() {
  assert(mark4_reader != NULL);
  assert(false);
  //mark4_reader->do_task();
}

template <class Type>
int
Input_node_tasklet_implementation<Type>::
goto_time(int time) {
  assert(false);
//  input_element->release();
//  int new_time = mark4_reader->goto_time(time, input_element);
//  if (time != new_time) {
//    DEBUG_MSG("New time " << time << " not found. Current time is " << new_time);
//  }
//  return new_time;
}

template <class Type>
bool
Input_node_tasklet_implementation<Type>::
append_time_slice(const Time_slice &time_slice) {
  if (!time_slices.empty()) {
    if (time_slices.end()->stop_time > time_slice.start_time) {
      DEBUG_MSG("Time slice starts before the end of the last time slice.");
      DEBUG_MSG("Not adding the time slice.");
      return false;
    }
  }
  
  time_slices.push_back(time_slice);

  return true;
}

#endif // INPUT_NODE_TASKLET_IMPL_H
