#include <iostream>

#ifdef ENABLE_TEST_UNIT

#include "Test_unit.h"
#include "backtrace.h"
#include "demangler.h"

int main(int argc, char** argv)
{
	Test_manager manager;

	std::cout << "Starting tests" << std::endl;
	manager.add_test( new Backtrace::Test() );

	manager.do_test();
}
#else
int main(int argc, char** argv)
{
		std::cout << "bootstrap version, test are disabled" << std::endl;
}
#endif //ENABLE_TEST_UNIT

