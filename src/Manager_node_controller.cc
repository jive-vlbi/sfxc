#include "Manager_node.h"

#include <assert.h>

Manager_node_controller::Manager_node_controller(Manager_node &node)
 : Controller(node), node(node)
{
}

Controller::Process_event_status 
Manager_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
    case MPI_TAG_CORRELATE_ENDED:
    {
      int i;
      MPI_Recv(&i, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      node.set_correlating_state(status.MPI_SOURCE, Manager_node::READY);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }

  return PROCESS_EVENT_STATUS_UNKNOWN;
}
