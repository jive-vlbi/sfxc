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

#include <Data_reader_file.h>
#include <Data_reader_tcp.h>

#include <Data_writer_file.h>
#include <Data_writer_tcp.h>
#include <Delay_table_akima.h>

#include <utils.h>

#include <assert.h>

#include <MPI_Transfer.h>

Correlator_node_controller::Correlator_node_controller(Correlator_node &node)
: Controller(node), node(node)
{
}

Correlator_node_controller::~Correlator_node_controller()
{
}

Controller::Process_event_status 
Correlator_node_controller::process_event(MPI_Status &status) {
  switch (status.MPI_TAG) {
    case MPI_TAG_DELAY_TABLE:
    {
      get_log_writer()(2) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      MPI_Transfer mpi_transfer;
      Delay_table_akima table;
      int sn;
      mpi_transfer.receive(status, table, sn);
      node.add_delay_table(sn, table);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
    case MPI_TAG_CORR_PARAMETERS:
    {
      get_log_writer()(2) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      Correlation_parameters parameters;
      MPI_Transfer::receive(status, parameters);

      node.start_correlating(parameters);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
return PROCESS_EVENT_STATUS_UNKNOWN;
}
