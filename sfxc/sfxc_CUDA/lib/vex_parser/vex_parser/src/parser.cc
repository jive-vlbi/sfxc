#include <generic.h>
#include <assert.h>

int main(int argc, char *argv[]) {
  if ( argc > 1 ) {
    std::cout << argv[1] << std::endl;
    yyin = fopen( argv[1], "r" );
    assert(yyin != NULL);
  }
  int result = yyparse ();

  return result;
}

