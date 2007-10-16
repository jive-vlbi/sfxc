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
  case MPI_TAG_TRACK_PARAMETERS: 
    {
      MPI_Transfer transfer;
      Track_parameters track_param;
      transfer.receive(status, track_param);
      node.set_track_parameters(track_param);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }


  case MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP: 
    {
      int32_t time_stamp;
      MPI_Recv(&time_stamp, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      time_stamp = node.get_time_stamp();
      MPI_Send(&time_stamp, 1, MPI_INT32, status.MPI_SOURCE, 
               MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP,
               MPI_COMM_WORLD);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_INPUT_NODE_GOTO_TIME:
    {
      int32_t new_time;
      MPI_Recv(&new_time, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      node.goto_time(new_time);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
      
    }
  case MPI_TAG_INPUT_NODE_STOP_TIME: 
    {
      int32_t stop_time;
      MPI_Recv(&stop_time, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      node.set_stop_time(stop_time);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_INPUT_NODE_ADD_TIME_SLICE:
    {
      int32_t message[4];
      MPI_Recv(&message, 4, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      node.add_time_slice(message[0],message[1],message[2],message[3]);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_GET_STATUS:
    {
      int32_t node_status;
      MPI_Recv(&node_status, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      node_status = node.get_status();
      MPI_Send(&node_status, 1, MPI_INT32, status.MPI_SOURCE, 
               MPI_TAG_GET_STATUS, MPI_COMM_WORLD);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }

  return PROCESS_EVENT_STATUS_UNKNOWN;
}
