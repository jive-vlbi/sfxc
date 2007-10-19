#ifndef TASKLETWORKER_HH
#define TASKLETWORKER_HH

#include <string>
#include "tasklet/thread.h"

class TaskletPool;

// A taskletworker execute tasks, they
// are implemented as system thread.
// A taskletworker is connected to a
// a taskletpool that provides the
// list of tasks to do.
class TaskletWorker : public Thread
{
public:
    // Constructor,
    // take a taskletpool and a tasklet name as argument
    TaskletWorker(const std::string name, TaskletPool& taskletpool);
    virtual ~TaskletWorker();

    // Accessor to name attribute
    const std::string& get_name();

    // This function is the main "thread" function.
    virtual void do_execute();
private:
    TaskletPool& m_taskpool;

    std::string m_name;
    bool m_running;

};

#endif // TASKLETWORKER_HH
