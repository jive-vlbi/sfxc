#include "Log_writer_void.h"
#include "Input_node.h"

int main() {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  try {
    Log_writer_void log_writer;
    Input_node input_node(0, 0, &log_writer);
  } catch (...) {
    assert(false);
  }

}
