#ifndef TASKLETWORKER_HH
#define TASKLETWORKER_HH

#include <string>
#include "tasklet/thread.h"

class Tasklet_pool;

// A taskletworker execute tasks, they
// are implemented as system thread.
// A taskletworker is connected to a
// a taskletpool that provides the
// list of tasks to do.
class Tasklet_worker : public Thread
{
public:
    // Constructor,
    // take a taskletpool and a tasklet name as argument
    Tasklet_worker(const std::string name, Tasklet_pool& taskletpool);
    virtual ~Tasklet_worker();

    // Accessor to name attribute
    const std::string& get_name();

    // This function is the main "thread" function.
    virtual void do_execute();
private:
    Tasklet_pool& m_taskpool;

    std::string m_name;
    bool m_running;

};

#endif // TASKLETWORKER_HH
