#include <iostream>
#include <cassert>
#include "Tasklet/Tasklet.h"
#include "Tasklet/Tasklet_worker.h"
#include "Tasklet/Tasklet_pool.h"
#include "Tasklet/Thread.h"

TaskletPool::TaskletPool(const std::string& name)
{
//   m_mutex = PTHREAD_MUTEX_INITIALIZER;
    m_name = name;
    add_worker("default_thread");
    pthread_mutex_init(&m_mutex, NULL);
}

TaskletPool::~TaskletPool()
{
    for(unsigned int i=0;i<m_vectorworker.size();i++)
    {
        delete m_vectorworker[i];
    }
}

void TaskletPool::run()
{
    for(unsigned int i=0;i<m_vectorworker.size();i++)
    {
            m_vectorworker[i]->start();
    }

}

void TaskletPool::add_worker(const std::string& name)
{
    //
    std::cout << "A worker [" << name << "] was added to pool [" << get_name() << "]" << std::endl;

    for(unsigned int i=0;i<m_vectorworker.size();i++)
    {
        assert( m_vectorworker[i]->get_name() != name );
    }

    TaskletWorker* worker = new TaskletWorker(name, *this);
    m_vectorworker.push_back(worker);
}

void TaskletPool::add_tasklet(Tasklet& tasklet)
{
    std::cout << "A tasklet [" << tasklet.desc() << "] was added to pool [" << get_name() << "]" << std::endl;
    m_vectortask.push_back(&tasklet);
    m_queuetask.push(&tasklet);
}

int TaskletPool::consume(Tasklet*& tasklet)
{
    lock();

    if( m_vectortask.size() == 0 ){ unlock(); return NO_MORE_TASK; }
    if( m_queuetask.empty() ){
        unlock();
        // This have to be replace here to use a pthread_cond_t
        // instead of an active loop;
        return NO_TASK_AVAILABLE;
    }

    tasklet = m_queuetask.front();
    m_queuetask.pop();

    unlock();
    return TASK_AVAILABLE;
}

const std::string& TaskletPool::get_name(){ return m_name; }

void TaskletPool::consumed(Tasklet& task)
{
    if( task.is_terminated() )
    {
        lock();
        remove_tasklet(task);
        unlock();
    }else{
        lock();
        m_queuetask.push(&task);
        unlock();
    }
}

void TaskletPool::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void TaskletPool::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

void TaskletPool::remove_tasklet(Tasklet& tasklet)
{
    m_vectortask.erase( std::remove(m_vectortask.begin(), m_vectortask.end(), &tasklet), m_vectortask.end() );
}
