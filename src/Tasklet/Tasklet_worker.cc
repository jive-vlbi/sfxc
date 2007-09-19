#include <iostream>
#include "Tasklet/Tasklet_worker.h"
#include "Tasklet/Tasklet_pool.h"
#include "Tasklet/Tasklet.h"

TaskletWorker::TaskletWorker(const std::string name, TaskletPool& taskletpool) :
    m_taskpool(taskletpool)
{
    m_name = name;
}



TaskletWorker::~TaskletWorker()
{

}

void TaskletWorker::do_execute()
{
    unsigned int retval;
    Tasklet* task;

    //std::cout << "After a long sleep the worker named [" << get_name() << "] wake up and start working..." << std::endl;
    while( (retval = m_taskpool.consume(task)) != NO_MORE_TASK ){
        if( retval == TASK_AVAILABLE ){
            std::cout << get_name() << " received a tasklet ::--->" << task->desc() << std::endl;
            task->do_task();
            m_taskpool.consumed(*task);
        }
    }
    //std::cout << "After a long working day the worker named [" << get_name() << "] fall asleep..." << std::endl;
}


const std::string& TaskletWorker::get_name(){ return m_name; }


