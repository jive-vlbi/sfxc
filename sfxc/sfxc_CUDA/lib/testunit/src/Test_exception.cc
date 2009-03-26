#include "Test_exception.h"

Test_exception::Test_exception(const std::string& src, const std::string& msg) {
  msg_ = msg;
  src_ = src;
  what_ = msg+":: "+src;
}

Test_exception::~Test_exception() throw() {}

String Test_exception::msg() {
  return src_+":: "+msg_;
}

const char* Test_exception::what() {
  return what_.c_str();
}
