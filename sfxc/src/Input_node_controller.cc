/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Input_node.h>

//---------------------------------------------------------------------------//
// Input_node_controller functions                                           //
//---------------------------------------------------------------------------//

Input_node_controller::Input_node_controller(Input_node &node) 
: Controller(node), node(node)
{
}
  
Controller::Process_event_status 
Input_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
    case MPI_TAG_INPUT_STREAM_SET_PRIORITY: {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));

      INT64 msg[3];
      MPI_Recv(&msg, 3, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

//      get_log_writer()
//        << "  - stream: " << msg[0]
//        << ", start: " << msg[1]
//        << ", stop: " << msg[2]
//        << std::endl;

      // stream, start, stop
      node.set_priority(msg[0],msg[1],msg[2]);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }

  return PROCESS_EVENT_STATUS_UNKNOWN;
}
