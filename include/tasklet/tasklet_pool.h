#ifndef TASKLET_POOL_H

#define TASKLET_POOL_H


#include <vector>
#include <queue>
#include <string>
#include <pthread.h>


class Tasklet;
class Tasklet_worker;

enum {TASK_AVAILABLE,  NO_TASK_AVAILABLE, NO_MORE_TASK};

//
// A tasklet pool contains a set of
// repetitive task to perform and a list of workers
// that realize the tasks.
//
class Tasklet_pool {

public:
  friend class Tasklet_worker;

  // Create a tasklet pool with a specific name
  Tasklet_pool(const std::string& name);

  virtual ~Tasklet_pool();

  const std::string& get_name();

  // Create and add a new worker to this pool
  void add_worker(const std::string& name);

  // Add a tasklet to the pool
  void add_tasklet(Tasklet& tasklet);

  // This function is used to
  // fire-up all the worker attached
  // to the task.
  void run();

private:
  void remove_tasklet(Tasklet& tasklet);

  // This class has to be thread-safe
  // as the worker may as for next-task
  // at the same time.
  void lock ();
  void unlock();

  // This fonction is call by Worker
  // to ask for a task to do. The given
  // tasklet is passed as a the parameter
  // the return value can be:
  // TASK_AVAILABLE: a task is available
  // NO_TASK_AVAILABLE: this means that all the
  //                    free to schedule task are
  //                    processed
  // NO_MORE_TASK: this means that all the pool
  //               has no more task and the worker
  //               can exit.
  int consume(Tasklet*& tasklet);

  // Use this function to finalize a task.
  void consumed(Tasklet&);


  std::string m_name;
  std::vector<Tasklet_worker*> m_vectorworker;

  // This queue contained the task available to
  // the worker.
  // It is possible to replace this by a vector
  // but this implies much more complex logics.
  std::queue<Tasklet*> m_queuetask;

  // This vector is used to store all the task
  // of the pool.
  std::vector<Tasklet*> m_vectortask;

  pthread_mutex_t m_mutex;
};







#endif // TASKLET_POOL_H

