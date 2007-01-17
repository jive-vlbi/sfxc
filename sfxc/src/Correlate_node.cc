#include "Correlate_node.h"

Correlate_node::Correlate_node(int rank, int buff_size)
 : Node(rank), 
   buffer(buff_size),
   log_writer(0,0),
   correlate_controller(buffer, log_writer),
   output_controller(buffer, log_writer)
{
  //write_debug(1, "Correlate_node(rank)");
  add_controller(&correlate_controller);
  add_controller(&output_controller);
}

Correlate_node::~Correlate_node()
{
  log_writer.message(1, "~Correlate_node()");
}

