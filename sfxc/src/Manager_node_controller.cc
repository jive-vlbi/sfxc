/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "Manager_node.h"

#include <utils.h>

Manager_node_controller::Manager_node_controller(Manager_node &node)
 : Controller(node), node(node)
{
}

Controller::Process_event_status 
Manager_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
    case MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED:
    {
      DEBUG_MSG("MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED");
      int correlator;
      MPI_Recv(&correlator, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      node.set_correlating_state(correlator, Manager_node::READY);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
    case MPI_TAG_DATASTREAM_EMPTY:
    {
      DEBUG_MSG("MPI_TAG_DATASTREAM_EMPTY");
      int32_t stream;
      MPI_Recv(&stream, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      DEBUG_MSG("Stream nr " << stream << " ended, terminating correlation");
      assert(false);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
    case MPI_TAG_OUTPUT_NODE_FINISHED:
    {
      DEBUG_MSG("MPI_TAG_OUTPUT_NODE_FINISHED");
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      node.end_correlation();

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }

  return PROCESS_EVENT_STATUS_UNKNOWN;
}
