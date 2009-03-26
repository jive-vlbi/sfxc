
#ifndef ENABLE_TEST_UNIT
#define ENABLE_TEST_UNIT
#endif //DEFINE ENABLE_TEST_UNIT

#include "network.h"

int main(int argc, char** argv) {
#ifdef ENABLE_TEST_UNIT
  Test_manager manager;
  //manager.add_test( new Data_writer_mem::Test() );
  //manager.add_test( new Data_reader_mark5::Test() );

  manager.do_test();
#endif // ENABLE_TEST_UNIT
}


