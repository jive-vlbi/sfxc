#ifndef TEST_ANINTERFACE_H

#define TEST_ANINTERFACE_H



#include "common.h"

#include "Testable.h"


template<class T>
class Test_aninterface : public Testable {
public:
  virtual ~Test_aninterface();

  void tests() {}

  static void tests(T*);
};



#endif // TEST_ANINTERFACE_H

