#ifndef TASKLET_HH
#define TASKLET_HH

#include <string>

typedef enum {TASKLET_FINISHED,
              TASKLET_CONTINUE,
              TASKLET_BLOCKED} TaskletState;

class Tasklet
{
public:
  virtual ~Tasklet();
  virtual void do_task() = 0;
  virtual bool has_work() = 0;
  virtual const char *name() = 0;
};


typedef Tasklet* pTasklet;



#endif // TASKLET_HH
