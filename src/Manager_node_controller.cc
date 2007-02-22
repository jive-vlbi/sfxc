#include "Manager_controller.h"
#include <assert.h>

Manager_controller::Manager_controller(Manager_node &node)
 : Controller(node.get_log_writer()), node(node)
{
}

Controller::Process_event_status 
Manager_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
    case MPI_TAG_CORRELATE_ENDED:
    {
      int i;
      int corr_node = status.MPI_SOURCE;
      MPI_Recv(&i, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      // NGHK: Do not correlate
      //node.set_start_time(node.get_stop_time());

      if (node.get_stop_time() <= node.get_start_time()) {
        // End program:
        int type=0;
        MPI_Send(&type, 1, MPI_INT32, 
                 status.MPI_SOURCE, MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
        node.add_number_correlator_node(-1);
        if (node.get_number_correlator_nodes()==0) {
          log_writer(0) << "number_correlator_nodes()==0" << std::endl;
          MPI_Send(&type, 1, MPI_INT32, 
                   0, MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
        }
      } else {
        log_writer(0) << "Start a new time slice\n";
        INT64 times[] = {node.get_start_time(), node.get_stop_time()};
        MPI_Send(times, 2, MPI_INT64, corr_node,
                 MPI_TAG_SET_TIME_SLICE, MPI_COMM_WORLD);
        node.set_start_time(node.get_stop_time());

        MPI_Send(&node.get_new_slice_number(), 1, MPI_INT32, corr_node,
                 MPI_TAG_START_CORRELATE_NODE, MPI_COMM_WORLD);
      }
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }

  return PROCESS_EVENT_STATUS_UNKNOWN;
}
