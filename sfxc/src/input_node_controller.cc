/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "input_node.h"
#include "mpi_transfer.h"

//---------------------------------------------------------------------------//
// Input_node_controller functions                                           //
//---------------------------------------------------------------------------//

Input_node_controller::Input_node_controller(Input_node &node)
    : Controller(node), node(node) {}

Controller::Process_event_status
Input_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;

  switch (status.MPI_TAG) {
  case MPI_TAG_TRACK_PARAMETERS: {
      MPI_Transfer transfer;
      Input_node_parameters input_node_param;
      transfer.receive(status, input_node_param);
      node.set_input_node_parameters(input_node_param);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }

  case MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP: {
      int64_t nticks;
      MPI_Recv(&nticks, 1, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      Time time_stamp = node.get_time_stamp();
      nticks = time_stamp.get_clock_ticks();
      MPI_Send(&nticks, 1, MPI_INT64, status.MPI_SOURCE,
               MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP,
               MPI_COMM_WORLD);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }

  case MPI_TAG_INPUT_NODE_SET_TIME: {
      int64_t times[3]; // start, stop and leave time
      MPI_Recv(&times[0], 3, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      Time start_time, stop_time, leave_time;
      start_time.set_clock_ticks(times[0]);
      stop_time.set_clock_ticks(times[1]);
      leave_time.set_clock_ticks(times[2]);
      node.add_time_interval(start_time, stop_time, leave_time);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_INPUT_NODE_ADD_TIME_SLICE: {
      int64_t message[4];
      Time slice_start, slice_stop;
      MPI_Recv(&message, 4, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      slice_start.set_clock_ticks(message[2]);
      slice_stop.set_clock_ticks(message[3]);
      node.add_time_slice_to_stream(message[0],message[1],slice_start,slice_stop);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_GET_STATUS: {
      int32_t node_status;
      MPI_Recv(&node_status, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      node_status = node.get_status();
      MPI_Send(&node_status, 1, MPI_INT32, status.MPI_SOURCE,
               MPI_TAG_GET_STATUS, MPI_COMM_WORLD);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_DELAY_TABLE: {
      Delay_table delay_table;
      int station;
      MPI_Transfer::receive(status, delay_table, station);
      node.set_delay_table(delay_table);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }

  return PROCESS_EVENT_STATUS_UNKNOWN;
}
