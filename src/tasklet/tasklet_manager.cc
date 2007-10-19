#include <iostream>
#include <cassert>
#include "tasklet/singleton.h"
#include "tasklet/thread.h"
#include "tasklet/tasklet_pool.h"
#include "tasklet/tasklet_worker.h"
#include "tasklet/tasklet_manager.h"

TaskletManager::TaskletManager()
{
  add_pool("default");
}


TaskletManager::~TaskletManager()
{
  for(unsigned int i=0;i<m_vectorpool.size();i++)
    {
      delete m_vectorpool[i];
    }
}

void TaskletManager::add_pool(const std::string name)
{
  for(unsigned int i=0;i<m_vectorpool.size();i++)
    {
      assert( m_vectorpool[i]->get_name() != name );
    }

  TaskletPool* taskletpool = new TaskletPool(name);
  m_vectorpool.push_back(taskletpool);
}

void TaskletManager::add_worker(const std::string& worker_name)
{
  add_worker_to_pool(worker_name, "default");
}

void TaskletManager::add_worker_to_pool(const std::string& worker_name, const std::string& pool_name)
{
  for(unsigned int i=0;i<m_vectorpool.size();i++)
    {
      if( m_vectorpool[i]->get_name() == pool_name )
        {
          m_vectorpool[i]->add_worker(worker_name);
          return;
        }
    }

  assert("pool not found");
}

void TaskletManager::add_tasklet(Tasklet& tasklet)
{
  add_tasklet_to_pool(tasklet, "default");
}

void TaskletManager::add_tasklet_to_pool(Tasklet& tasklet, const std::string &pool_name)
{
  for(unsigned int i=0;i<m_vectorpool.size();i++)
    {
      if( m_vectorpool[i]->get_name() == pool_name )
        {
          m_vectorpool[i]->add_tasklet(tasklet);
          return;
        }
    }

  assert("pool not found");
}

TaskletPool &TaskletManager::get_pool(const std::string &pool_name)
{
  for(unsigned int i=0;i<m_vectorpool.size();i++)
    {
      if( m_vectorpool[i]->get_name() == pool_name )
        {
          return *m_vectorpool[i];
        }
    }
  
  assert("pool not found");
  throw(-1);
}
TaskletPool &TaskletManager::get_pool()
{
  return *m_vectorpool[0];
}

void TaskletManager::run()
{
  for(unsigned int i=0;i<m_vectorpool.size();i++)
    {
      m_vectorpool[i]->run();
    }

  singleton<ThreadPool>::instance().wait_for_all_termination();
}
