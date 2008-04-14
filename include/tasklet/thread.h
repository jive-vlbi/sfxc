#ifndef THREAD_HH
#define THREAD_HH

#include <vector>
#include <pthread.h>

class Thread {
  friend class ThreadPool;
  pthread_t m_threadid;
public:
  Thread();
  virtual ~Thread();

  void start();
  static void* execute(void*);
  virtual void do_execute() = 0;
};

class ThreadPool {
  friend class Thread;
public:
  ThreadPool();
  void register_thread(Thread& thread);
  void wait_for_all_termination();
private:
  std::vector<Thread*> m_vectorthread;
};




#endif // THREAD_HH
