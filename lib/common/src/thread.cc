#include <iostream>
#include <pthread.h>
#include <cassert>

#include "demangler.h"
#include "exception_common.h"
#include "singleton.h"
#include "thread.h"
#include "signal_handler.h"

class ThreadException : public Exception
{
public:
    ThreadException(const std::string& string, Backtrace& bt) :
        Exception(string, bt){}

    virtual ~ThreadException() throw () {};

    virtual const String type()
    {
				return get_type_name(this);
    }
};


ThreadPool::ThreadPool()
{

}
void ThreadPool::s_wait_for_all_termination(){
  singleton<ThreadPool>::instance().wait_for_all_termination();
}

void ThreadPool::register_thread(Thread& thread)
{
        m_vectorthread.push_back(&thread);
}

void ThreadPool::wait_for_all_termination()
{
    for(unsigned int i=0;i<m_vectorthread.size();i++)
    {
        void *retval;
        pthread_join( (m_vectorthread[i]->m_threadid), &retval );
        std::cout << "Thread number " << i << " is terminated" << std::endl;
    }
    std::cout << " Normal thread termination " << m_vectorthread.size() << " threads" << std::endl;
}

ThreadPool& operator||( Thread& a, Thread& b)
{
    ThreadPool *g=new ThreadPool();
    g->register_thread( a );
    g->register_thread( b );

    a.start();
    b.start();

    return *g;
}

ThreadPool& operator||( ThreadPool& g, Thread& b)
{
    g.register_thread( b );
    b.start();
    return g;
}

void wait(ThreadPool& pool)
{
    pool.wait_for_all_termination();
}


Thread::Thread()
{
    singleton<ThreadPool>::instance().register_thread(*this);
}

Thread::~Thread()
{

}

Thread& Thread::start()
{
    int rc = pthread_create(&m_threadid, NULL, execute, (void *)this);
    if(rc){
        Backtrace bt(__PRETTY_FUNCTION__);
        throw ThreadException("Unable to create a thread", bt);
    }
    return *this;
}

void Thread::stop()
{
  isrunning_ = false;
}

void* Thread::execute(void *param)
{
    Signal_handler::install();
    Thread *th = static_cast<Thread*>(param);
    assert( th != NULL );
    try{
        th->isrunning_ = true;
        th->do_execute();
    }catch(Exception& e){
        //LOG2( singleton<Log_writer_cout>::instance(), e);
				std::cerr << e << std::endl;
        throw e;
    }

    return NULL;
}
