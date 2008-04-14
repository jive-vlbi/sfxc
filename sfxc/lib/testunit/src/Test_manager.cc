#include "Test_manager.h"


Test_manager::Test_manager() {
  //ctor
}

Test_manager::~Test_manager() {
  //dtor
}

void Test_manager::add_test(pTestable test) {
  vectortests_.push_back(test);
  int toto;
}

void Test_manager::do_test() {
  for (unsigned int i=0;i<vectortests_.size();i++) {
    vectortests_[i]->test();
  }
}
