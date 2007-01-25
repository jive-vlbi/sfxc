#include "Manager_controller.h"
#include <assert.h>

Manager_controller::Manager_controller(Log_writer &writer, int numtasks)
 : Controller(writer), numtasks(numtasks), slice(0)
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
      MPI_Recv(&i, 1, MPI_INT, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      log_writer(0) << "Start a new time slice\n";

      if (stop <= start) {
        // End program:
        for (int node=0; node<numtasks; node++) {
//          sleep(10);
//          int type = MPI_TAG_CORRELATION_READY;
//          MPI_Send(&type, 1, MPI_INT, node, MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
        }
      } else {
        INT64 times[] = {start, stop};
        MPI_Send(times, 2, MPI_LONG, corr_node,
                 MPI_TAG_SET_TIME_SLICE, MPI_COMM_WORLD);
        start = stop;

        MPI_Send(&slice, 1, MPI_INT, corr_node,
                 MPI_TAG_START_CORRELATE_NODE, MPI_COMM_WORLD);
        slice++;
      }
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }

  return PROCESS_EVENT_STATUS_UNKNOWN;
}
