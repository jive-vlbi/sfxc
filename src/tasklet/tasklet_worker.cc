#include <iostream>
#include "tasklet/tasklet_worker.h"
#include "tasklet/tasklet_pool.h"
#include "tasklet/tasklet.h"

Tasklet_worker::Tasklet_worker(const std::string name, Tasklet_pool& taskletpool) :
    m_taskpool(taskletpool) {
  m_name = name;
}



Tasklet_worker::~Tasklet_worker() {
}

void Tasklet_worker::do_execute() {
  unsigned int retval;
  Tasklet* task;

  //std::cout << "After a long sleep the worker named [" << get_name() << "] wake up and start working..." << std::endl;
  while ( (retval = m_taskpool.consume(task)) != NO_MORE_TASK ) {
    if ( retval == TASK_AVAILABLE ) {
      task->do_task();
      m_taskpool.consumed(*task);
    }
  }
  //std::cout << "After a long working day the worker named [" << get_name() << "] fall asleep..." << std::endl;
}


const std::string& Tasklet_worker::get_name() {
  return m_name;
}


