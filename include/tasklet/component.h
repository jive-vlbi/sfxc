#ifndef COMPONENT_H
#define COMPONENT_H

#include "tasklet/tasklet.h"
#include "tasklet/thread.h"
#include "tasklet/tasklet_pool.h"

class Component : public Tasklet, public Thread {
public:
  Component(TaskletPool &work_queue);

  TaskletPool &work_queue;
};


#endif // COMPONENT_H
