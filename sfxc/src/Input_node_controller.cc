/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Input_node.h>
#include <MPI_Transfer.h>
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
  case MPI_TAG_INPUT_NODE_INPUT_STREAM_SET_PRIORITY: 
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));

      int64_t msg[4];
      MPI_Recv(&msg, 4, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      // stream, start, stop
      assert((int64_t)(int(msg[1])) == msg[1]);
      node.set_priority(msg[0],msg[1],msg[2],msg[3]);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_CONTROL_PARAM: 
    {
      MPI_Transfer mpi_transfer;
      mpi_transfer.receive_general_parameters(status, 
                                              node.RunPrms, node.GenPrms, node.StaPrms);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP: 
    {
      int64_t time_stamp;
      MPI_Recv(&time_stamp, 1, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      time_stamp = node.get_time_stamp();
      MPI_Send(&time_stamp, 1, MPI_INT64, status.MPI_SOURCE, 
               MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP,
               MPI_COMM_WORLD);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_INPUT_NODE_GOTO_TIME:
    {
      int64_t new_time;
      MPI_Recv(&new_time, 1, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      node.goto_time(new_time);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
      
    }
  case MPI_TAG_INPUT_NODE_STOP_TIME: 
    {
      int64_t stop_time;
      MPI_Recv(&stop_time, 1, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      node.set_stop_time(stop_time);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }

  return PROCESS_EVENT_STATUS_UNKNOWN;
}
