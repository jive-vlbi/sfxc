#include <iostream>
#include <cassert>
#include "tasklet/singleton.h"
#include "tasklet/thread.h"
#include "tasklet/tasklet_pool.h"
#include "tasklet/tasklet_worker.h"
#include "tasklet/tasklet_manager.h"

Tasklet_manager::Tasklet_manager() {
  add_pool("default");
}


Tasklet_manager::~Tasklet_manager() {
  for (unsigned int i=0;i<m_vectorpool.size();i++) {
    delete m_vectorpool[i];
  }
}

void Tasklet_manager::add_pool(const std::string name) {
  for (unsigned int i=0;i<m_vectorpool.size();i++) {
    assert( m_vectorpool[i]->get_name() != name );
  }

  Tasklet_pool* taskletpool = new Tasklet_pool(name);
  m_vectorpool.push_back(taskletpool);
}

void Tasklet_manager::add_worker(const std::string& worker_name) {
  add_worker_to_pool(worker_name, "default");
}

void Tasklet_manager::add_worker_to_pool(const std::string& worker_name, const std::string& pool_name) {
  for (unsigned int i=0;i<m_vectorpool.size();i++) {
    if ( m_vectorpool[i]->get_name() == pool_name ) {
      m_vectorpool[i]->add_worker(worker_name);
      return;
    }
  }

  assert("pool not found");
}

void Tasklet_manager::add_tasklet(Tasklet& tasklet) {
  add_tasklet_to_pool(tasklet, "default");
}

void Tasklet_manager::add_tasklet_to_pool(Tasklet& tasklet, const std::string &pool_name) {
  for (unsigned int i=0;i<m_vectorpool.size();i++) {
    if ( m_vectorpool[i]->get_name() == pool_name ) {
      m_vectorpool[i]->add_tasklet(tasklet);
      return;
    }
  }

  assert("pool not found");
}

Tasklet_pool &Tasklet_manager::get_pool(const std::string &pool_name) {
  for (unsigned int i=0;i<m_vectorpool.size();i++) {
    if ( m_vectorpool[i]->get_name() == pool_name ) {
      return *m_vectorpool[i];
    }
  }

  assert("pool not found");
  throw(-1);
}
Tasklet_pool &Tasklet_manager::get_pool() {
  return *m_vectorpool[0];
}

void Tasklet_manager::run() {
  for (unsigned int i=0;i<m_vectorpool.size();i++) {
    m_vectorpool[i]->run();
  }

  singleton<ThreadPool>::instance().wait_for_all_termination();
}
