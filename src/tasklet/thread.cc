#include <iostream>
#include <pthread.h>
#include <cassert>
#include "tasklet/singleton.h"
#include "tasklet/thread.h"

ThreadPool::ThreadPool()
{

}

void ThreadPool::register_thread(Thread& thread)
{
        m_vectorthread.push_back(&thread);
}

void ThreadPool::wait_for_all_termination()
{
    std::cout << " Wait for termination of " << m_vectorthread.size() << " threads" << std::endl;
    for(unsigned int i=0;i<m_vectorthread.size();i++)
    {
        void *retval;
        pthread_join( (m_vectorthread[i]->m_threadid), &retval );
        std::cout << "Thread number " << i << " is terminated" << std::endl;
    }
}


Thread::Thread()
{
    singleton<ThreadPool>::instance().register_thread(*this);
}

Thread::~Thread()
{

}

void Thread::start()
{
    int rc = pthread_create(&m_threadid, NULL, execute, (void *)this);
    if(rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
}

void* Thread::execute(void *param)
{
    Thread *th = static_cast<Thread*>(param);
    assert( th != NULL );
    th->do_execute();
    return NULL;
}
