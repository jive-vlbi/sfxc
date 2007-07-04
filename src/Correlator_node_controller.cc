/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Correlator_node.h>
#include <signal.h>
#include <iostream>
#include <InData.h>

#include <Data_reader_file.h>
#include <Data_reader_tcp.h>

#include <Data_writer_file.h>
#include <Data_writer_tcp.h>

#include <utils.h>

#include <assert.h>

#include <MPI_Transfer.h>

Correlator_node_controller::Correlator_node_controller(Correlator_node &node)
 : Controller(node), 
   node(node)
{
   GenPrms.set_usStart(0);
}

Correlator_node_controller::~Correlator_node_controller()
{
}

Controller::Process_event_status 
Correlator_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_CORRELATE_TIME_SLICE:
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
      int64_t time[3]; // slice number, start, duration
      MPI_Recv(&time, 3, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

#ifndef USE_FILES_FOR_OUTPUT
      int size_of_slice = -1;
      int64_t priority[] = 
        {node.get_correlate_node_number(), 
         time[0],
         size_of_slice
        };
      MPI_Send(&priority, 3, MPI_INT64, 
               RANK_OUTPUT_NODE, MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY, 
               MPI_COMM_WORLD);
#endif
      
      node.start_correlating(time[1], time[2]);
      node.set_slice_number(time[0]);           

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_CONTROL_PARAM:
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
      MPI_Transfer mpi_transfer;
      mpi_transfer.receive_general_parameters(status,RunPrms,GenPrms,StaPrms);
      
      node.set_parameters(RunPrms, GenPrms, StaPrms);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_DELAY_TABLE:
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
      MPI_Transfer mpi_transfer;
      DelayTable table;
      int sn;
      mpi_transfer.receive_delay_table(status, table, sn);
      node.add_delay_table(sn, table);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}
