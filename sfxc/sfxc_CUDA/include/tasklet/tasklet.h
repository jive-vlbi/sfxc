#ifndef TASKLET_HH
#define TASKLET_HH

#include <string>

typedef enum {TASKLET_FINISHED,
              TASKLET_CONTINUE,
              TASKLET_BLOCKED} TaskletState;

/**
 * Generic tasklet. Every unitary bit of functionality is grouped in a tasklet.
 * This makes it possible to run different tasklets in different threads
 * \ingroup ImportantClasses
 **/
class Tasklet {
public:
  virtual ~Tasklet();

  /**
   * Execute the task, process a bit of work. 
   * This function should not be blocking.
   * \pre has_work()
   **/
  virtual void do_task() = 0;

  /**
  * Check whether there is work for the tasklet.
  * This function should not be blocking.
  **/
  virtual bool has_work() = 0;

  /**
  * Name of the tasklet for debugging purposes
  **/
  virtual const char *name() = 0;
};


typedef Tasklet* pTasklet;



#endif // TASKLET_HH
