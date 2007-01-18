#include "Correlator_node.h"
#include "ProcessData.h"

Correlator_node::Correlator_node(int rank, int buff_size)
 : Node(rank), 
   buffer(buff_size),
   correlator_controller(buffer, log_writer),
   output_controller(buffer, log_writer)
{
  log_writer.MPI(0, "Correlate_node(rank)");
  add_controller(&correlator_controller);
  add_controller(&output_controller);
  set_log_writer(log_writer);
}

Correlator_node::~Correlator_node()
{
  log_writer.message(1, "~Correlate_node()");
}

