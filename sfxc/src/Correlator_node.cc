#include "Correlator_node.h"

Correlator_node::Correlator_node(int rank, int buff_size)
 : Node(rank), 
   buffer(buff_size),
   log_writer(0,0),
   correlator_controller(buffer, log_writer),
   output_controller(buffer, log_writer)
{
  //write_debug(1, "Correlate_node(rank)");
  add_controller(&correlator_controller);
  add_controller(&output_controller);
}

Correlator_node::~Correlator_node()
{
  log_writer.message(1, "~Correlate_node()");
}

