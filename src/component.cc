#include "tasklet/component.h"

Component::Component(TaskletPool &work_queue)
  : work_queue(work_queue) {
}
