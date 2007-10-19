#ifndef TASKLETPOOL_HH
#define TASKLETPOOL_HH

#include <vector>
#include <queue>
#include <string>
#include <pthread.h>

class Tasklet;
class TaskletWorker;

typedef enum {TASK_AVAILABLE,  NO_TASK_AVAILABLE, NO_MORE_TASK};

//
// A tasklet pool contains a set of
// repetitive task to perform and a list of workers
// that realize the tasks.
//
class TaskletPool
{

public:
    friend class TaskletWorker;

    // Create a tasklet pool with a specific name
    TaskletPool(const std::string& name);

    virtual ~TaskletPool();

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
    std::vector<TaskletWorker*> m_vectorworker;

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





#endif // TASKLETPOOL_HH
