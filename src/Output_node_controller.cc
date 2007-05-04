/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */


// Output_node_controller is defined in Output_node.h:
#include <Output_node.h>

#include <iostream>
#include <assert.h>

#include <Data_writer_file.h>
#include <Data_reader_tcp.h>
#include <TCP_Connection.h>
#include <Queue_buffer.h>

Output_node_controller::Output_node_controller(Output_node &node)
  : Controller(node), node(node) {
    
}

Controller::Process_event_status
Output_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_OUTPUT_STREAM_SET_PRIORITY:
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
      INT64 weight[2];
      MPI_Recv(&weight, 2, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      // Create an output buffer:
      node.create_buffer(weight[0]);
      node.set_weight_of_input_stream(weight[0], weight[1]);
      node.set_status();
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED:
    {
//      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
      UINT64 rank[2];
      MPI_Recv(&rank, 2, MPI_UINT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      get_log_writer()(0) << print_MPI_TAG(status.MPI_TAG)
                          << " From: corr.node " << rank[0] <<", bytes " << rank[1] << std::endl;;

      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      node.time_slice_finished(rank[0], rank[1]); 
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_OUTPUT_NODE_CORRELATION_READY:
    {
      get_log_writer().MPI(0, print_MPI_TAG(status.MPI_TAG));
      INT32 nr_of_time_slices;
      MPI_Recv(&nr_of_time_slices, 2, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      node.set_number_of_time_slices(nr_of_time_slices);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}
