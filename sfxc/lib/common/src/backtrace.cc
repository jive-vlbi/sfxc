#include <string.h>
#include <stdlib.h>
#include "backtrace.h"
#include "demangler.h"

void get_trace (Vector_string& trace_) {
  void *array[50];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace (array, 50);
  strings = backtrace_symbols (array, size);

//printf ("Obtained %zd stack frames.\n", size);

  for (i = 1; i < size; i++) {
    //Demangler demangler( strings[i] );
    //printf ("This is a %s\n", strings[i] );
    char *ptr = strings[i];

    strtok_r( ptr, "()", &ptr);
    char *tok2 = strtok_r( ptr, "()", &ptr);
    strtok_r( ptr, "()", &ptr);

    char *tok2bis = strtok_r( tok2, "+", &ptr);
    //printf("hello: %s et aussi: %s", tok2, tok2bis);

    if ( tok2bis != NULL ) {
      if ( strcmp( tok2bis, "main" ) ) {
        Demangler demangle(tok2bis);
        //std::cout << " Demangler: " << demangle.value() << std::endl;
        trace_.push_back( demangle.value() );
      } else {
        i = size;
        trace_.push_back( "main()" );
      }
    } else {
      trace_.push_back( strings[i] );
    }
  }

  free (strings);
}

Backtrace::Backtrace(const Backtrace& bt) {
  trace_ = bt.trace_;
}

Backtrace::Backtrace(const std::string& trace) {
#ifdef ENABLE_BACKTRACE
  get_trace( trace_ );
#endif //ENABLE_BACKTRACE
  trace_.push_back(trace);
}

Backtrace::Backtrace() {
#ifdef ENABLE_BACKTRACE
  get_trace( trace_ );
#endif //ENABLE_BACKTRACE
}

std::ostream& operator<<(std::ostream& out, const Backtrace& backtrace) {
  for (unsigned int i=1;i<backtrace.trace_.size();i++) {
    out << "[" << i << "] " << backtrace.trace_[i]  << std::endl;
  }
  return out;
}

/*
#ifdef ENABLE_TEST_UNIT
void Backtrace::Test::tests()
{
 std::cout << "Backtrace: starting testing " << std::endl;

 Backtrace backtrace;
 std::cout << "Dislay backtrace: \n" << backtrace << std::endl;
}
#endif // ENABLE_TEST_UNIT
*/
