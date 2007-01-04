#include "Correlate_node.h"

Correlate_node::Correlate_node(int rank, int buff_size)
 : Node(rank), 
   buffer(buff_size),
   correlate_controller(buffer),
   output_controller(buffer)
{
  //write_debug(1, "Correlate_node(rank)");
  add_controller(&correlate_controller);
  add_controller(&output_controller);
}

Correlate_node::~Correlate_node()
{
  write_debug(1, "~Correlate_node()");
}

