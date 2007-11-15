#include <iostream>
#include <cassert>
#include "tasklet/tasklet.h"
#include "tasklet/tasklet_worker.h"
#include "tasklet/tasklet_pool.h"
#include "tasklet/thread.h"

Tasklet_pool::Tasklet_pool(const std::string& name)
{
//   m_mutex = PTHREAD_MUTEX_INITIALIZER;
    m_name = name;
    add_worker("default_thread");
    pthread_mutex_init(&m_mutex, NULL);
}

Tasklet_pool::~Tasklet_pool()
{
    for(unsigned int i=0;i<m_vectorworker.size();i++)
    {
        delete m_vectorworker[i];
    }
}

void Tasklet_pool::run()
{
    for(unsigned int i=0;i<m_vectorworker.size();i++)
    {
            m_vectorworker[i]->start();
    }

}

void Tasklet_pool::add_worker(const std::string& name)
{
    for(unsigned int i=0;i<m_vectorworker.size();i++)
    {
        assert( m_vectorworker[i]->get_name() != name );
    }

    Tasklet_worker* worker = new Tasklet_worker(name, *this);
    m_vectorworker.push_back(worker);
}

void Tasklet_pool::add_tasklet(Tasklet& tasklet)
{
    m_vectortask.push_back(&tasklet);
    m_queuetask.push(&tasklet);
}

int Tasklet_pool::consume(Tasklet*& tasklet)
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

const std::string& Tasklet_pool::get_name(){ return m_name; }

void Tasklet_pool::consumed(Tasklet& task)
{
}

void Tasklet_pool::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void Tasklet_pool::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

void Tasklet_pool::remove_tasklet(Tasklet& tasklet)
{
    m_vectortask.erase( std::remove(m_vectortask.begin(), m_vectortask.end(), &tasklet), m_vectortask.end() );
}
