#include "Test_unit.h"
#include "threadsafe_queue.h"
#include "memory_pool.h"

int main(int argc, char** argv) {
#ifdef ENABLE_TEST_UNIT
  Test_manager manager;

  Threadsafe_queue<int> queue;
  manager.add_test( new Threadsafe_queue<int>::Test() );

  Memory_pool<int> buffer(1);
  manager.add_test( new Memory_pool<int>::Test() );

  manager.do_test();
#endif // ENABLE_TEST_UNIT
}


