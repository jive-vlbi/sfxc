#include <iostream>
#include <pthread.h>
#include <cassert>

#include "demangler.h"
#include "exception_common.h"
#include "singleton.h"
#include "thread.h"
#include "signal_handler.h"

class ThreadException : public Exception {
public:
  ThreadException(const std::string& string, Backtrace& bt) :
      Exception(string, bt) {}

  virtual ~ThreadException() throw () {};

  virtual const String type() {
    return get_type_name(this);
  }
};



void wait(Thread& thread) {
  void *retval;
  pthread_join( thread.m_threadid, &retval );
}


ThreadPool::ThreadPool() {}
void ThreadPool::s_wait_for_all_termination() {
  singleton<ThreadPool>::instance().wait_for_all_termination();
}

void ThreadPool::register_thread(Thread& thread) {
  m_vectorthread.push_back(&thread);
}



void ThreadPool::wait_for_all_termination() {
  for (unsigned int i=0;i<m_vectorthread.size();i++) {
    void *retval;

    CHECK_ZERO( pthread_join( (m_vectorthread[i]->m_threadid), &retval ) );
    //std::cout << "Thread number " << i << " is terminated" << std::endl;
  }
  //std::cout << " Normal thread termination " << m_vectorthread.size() << " threads" << std::endl;
}

void ThreadPool::start_all() {
  for (unsigned int i=0;i<m_vectorthread.size();i++) {
    m_vectorthread[i]->start();
  }
}

void ThreadPool::stop_all() {
  for (unsigned int i=0;i<m_vectorthread.size();i++) {
    //std::cout << "Stopping thread:" << i << std::endl;
    m_vectorthread[i]->stop();
  }
}

bool ThreadPool::still_running() {
  bool is_still_running = false;
  for (unsigned int i=0;i<m_vectorthread.size();i++) {
    if(m_vectorthread[i]->isrunning_ == true)
      is_still_running = true;
  }
  return is_still_running;
}

ThreadPool& operator||( Thread& a, Thread& b) {
  ThreadPool *g=new ThreadPool();
  g->register_thread( a );
  g->register_thread( b );

  a.start();
  b.start();

  return *g;
}

ThreadPool& operator||( ThreadPool& g, Thread& b) {
  g.register_thread( b );
  b.start();
  return g;
}

ThreadPool& operator<<( ThreadPool& g, Thread& b) {
  g.register_thread( b );
  b.start();
  return g;
}


void wait(ThreadPool& pool) {
  pool.wait_for_all_termination();
}


Thread::Thread() {
  singleton<ThreadPool>::instance().register_thread(*this);
}

Thread::~Thread() {
// @todo (damien#9#): The threads are not correctly unregistred from the singleton<ThreadPool>. This may cause crash !
}

Thread& Thread::start() {
  CHECK_ZERO( pthread_create(&m_threadid, NULL, execute, (void *)this) );
  CHECK_ZERO( pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) );
  set_cancel_state(true);

  return *this;
}

void Thread::stop() {
  isrunning_ = false;
  CHECK_ZERO( pthread_cancel(m_threadid) );
}

void* Thread::execute(void *param) {
  Signal_handler::install();
  Thread *th = static_cast<Thread*>(param);
  assert( th != NULL );
  try {
    th->isrunning_ = true;
    th->do_execute();
  } catch (Exception& e) {
    std::cerr << "Exception received from a thread: " << std::endl;
    std::cerr << " the thread is interrupted !" << std::endl;
    std::cerr << e << std::endl;
    throw e;
  }

  return NULL;
}

void Thread::set_cancel_state(bool state) {
  if ( state == true ) {
    CHECK_ZERO( pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL ) );
  } else {
    CHECK_ZERO( pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, NULL ) );
  }
}
