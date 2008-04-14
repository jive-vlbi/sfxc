#ifndef TEST_MANAGER_H

#define TEST_MANAGER_H



#include <vector>
#include "Testable.h"


class Test_manager {
  std::vector<pTestable> vectortests_;
public:
  Test_manager();
  ~Test_manager();
  void add_test(pTestable test);
  void do_test();
};



#endif // TEST_MANAGER_H

