#ifndef TEST_ACLASS_H
#define TEST_ACLASS_H

#include <stdexcept>
#include <iostream>

#include "exception_common.h"
#include "Testable.h"
#include "Test_exception.h"
#include "demangler.h"

template<class T>
class Test_aclass : public Testable {
public:
  virtual ~Test_aclass() {};

  void test() {
    std::cout << "========== Testing class: " << get_type_name<T>() << " :========== "<< std::endl;
    try {
      tests();
    } catch (Exception& e) {
      std::cout << "Unexpected Exception received during class testing:" << std::endl;
      std::cout << e << std::endl;
    } catch (Test_exception& e) {
      std::cout << "Unexpected Test_exception received during class testing:" << std::endl;
      std::cout << " message is: " << e.what() << std::endl;
    } catch (std::exception& e) {
      std::cout << "Unexpected std::exception received during class testing:" << std::endl;
      std::cout << " message is: " << e.what() << std::endl;
    }
  }


  virtual void tests() = 0;
};

#endif // TEST_ACLASS_H
