#include <stdlib.h>
#include "common.h"

int& operator<<(int& dest, const String& s) {
  //std::stringstream ss(s);
  //dest << ss;
  dest = atoi(s.c_str());
  return dest;
}
