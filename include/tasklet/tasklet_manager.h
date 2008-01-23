#ifndef TASKLETMANAGER_HH
#define TASKLETMANAGER_HH

#include <vector>

class Tasklet;
class Tasklet_pool;
class Tasklet_worker;

//
// A tasklet manager handles the differents
// task that composed an application to the
// threads. The manager is composed of
// pool's of task that can be executed
// by "workers"(threads). Several workers
// can act on the same taskpool.
// Remark: the tasks of the system are independant
// and it is a bad idea to make them interacting
// except by using using thread-safe
// "man in the middle" (like Buffers, Message-Passing).
class Tasklet_manager
{
public:
    Tasklet_manager();
    ~Tasklet_manager();


    // Add a worker to the default pool
    void add_worker(const std::string& worker_name);

    // Add a tasklet to the default pool
    // To avoid concurency issues do not put the same
    // tasklet is several pool.
    void add_tasklet(Tasklet& tasklet);

    // Add a new pool to the pool list
    // The function return an Exception if a pool with the
    // same "name" already exist.
    void add_pool(const std::string name);

    // Add a new worker to the pool
    // The function rise an Exception if the pool already contains a
    // worker with the same name.
    void add_worker_to_pool(const std::string& worker_name, const std::string& pool_name);

  Tasklet_pool &get_pool();
  Tasklet_pool &get_pool(const std::string &name);

    // Add a tasklet to a pool.
    void add_tasklet_to_pool(Tasklet& tasklet, const std::string &name);

    // Use this function to init the taskmanager
    void init();

    // Use this function fire up all the worker.
    void run();

private:
    std::vector<Tasklet_pool*> m_vectorpool;
};




#endif // TASKLETMANAGER_HH
