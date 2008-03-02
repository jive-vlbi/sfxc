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

class Thread
{
    friend class ThreadPool;
    pthread_t m_threadid;
    bool isrunning_;
public:
    Thread();
    virtual ~Thread();

    Thread& start();
    void stop();
    
    static void* execute(void*);
    virtual void do_execute() = 0;
};

class ThreadPool
{
    friend class Thread;
    public:
        ThreadPool();
        void register_thread(Thread& thread);
        void wait_for_all_termination();

        static void s_wait_for_all_termination();
    private:
        std::vector<Thread*> m_vectorthread;
};

ThreadPool& operator||( Thread& a, Thread& b);
ThreadPool& operator||( ThreadPool& g, Thread& b);
void wait(ThreadPool& pool);



#endif // THREAD_HH
