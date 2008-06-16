/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof  <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 *  This file contains:
 *     - definition of the Thread class around posix-thread API.
 */
#ifndef THREAD_HH
#define THREAD_HH

#include <vector>
#include <pthread.h>


/*******************************************************************************
*
* @class Thread
* @author Damien Marchal
* @desc A trivial implementation of thread as a class.
* A class can be transformed to a thread by inheriting from Thread and
* implementing the void do_execute() virtual function.
* The thread can then be started/stopped. The thread is running while
* the do_execute() function is running.
*******************************************************************************/
class Thread {
public:
  /*****************************************************************************
  * start the execution of the thread, set the isrunning_ variable to true.
  * the return Thread& is a reference to the thread itself.
  *****************************************************************************/
  Thread& start();

  /*****************************************************************************
  * set the isrunning_ variable to false. This variable can then be checked
  * by the user defined do_execute function for early exit.
  *****************************************************************************/
  void stop();

  /*****************************************************************************
  *
  * Example:
  * void do_execute(){
  *   while( isrunning_ ){
  *      sleep(1);
  *      std::cout << "Please let me sleep more !" << std::endl;
  *   }
  * }
  *
  *****************************************************************************/
  virtual void do_execute() = 0;

	void set_cancel_state(bool state);

protected:
  Thread();
  virtual ~Thread();

  // indicate if the thread is running. this variable is changed
  // after a call to the stop function.
  bool isrunning_;

private:
  static void* execute(void*);

  friend class ThreadPool;
  friend void wait(Thread&);

  pthread_t m_threadid;
};


/*******************************************************************************
*
* @class ThreadPool
* @author Damien Marchal
* @desc A ThreadPool is a groupe of thread
* This class can be use to store a groupe of thread that are running.
*******************************************************************************/
class ThreadPool {
  friend class Thread;
public:
  ThreadPool();

  /*****************************************************************************
  * register a new thread to the threadpool.
  *****************************************************************************/
  void register_thread(Thread& thread);

  /*****************************************************************************
  * Wait for all thread in the threadpool to be terminated to continue the
  * execution.
  *****************************************************************************/
  void wait_for_all_termination();

  /*****************************************************************************
  * All threads are, by default, stored in a static ThreadPool. Use this
  * to wait normal termination of all of them
  *****************************************************************************/
  static void s_wait_for_all_termination();

  /*****************************************************************************
  * Request to stop all thread in a thread pool.
  *****************************************************************************/
  void stop_all();

  /*****************************************************************************
  * Request to start all thread in a thread pool.
  *****************************************************************************/
  void start_all();

  /*****************************************************************************
  * return true if there is still a least one thread running.
  *****************************************************************************/
  bool still_running();
private:
  std::vector<Thread*> m_vectorthread;
};

ThreadPool& operator||( Thread& a, Thread& b);
ThreadPool& operator||( ThreadPool& g, Thread& b);
ThreadPool& operator<<( ThreadPool& g, Thread& b);
void wait(ThreadPool& pool);
void wait(Thread& thread);


template<class Th>
class SimpleThread : public Thread
{
	Th& object_;

	public:
		SimpleThread(Th& object) : object_(object) {};

		void do_execute()
		{
			while(isrunning_){
					object_.do_task();
			}
		}
};
#endif // THREAD_HH
