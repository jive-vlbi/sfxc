/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "correlator_node.h"
#include "data_reader_file.h"
#include "data_reader_tcp.h"

#include "data_writer_file.h"
#include "data_writer_tcp.h"
#include "delay_table_akima.h"
#include "uvw_model.h"

#include "utils.h"

#include "mpi_transfer.h"

#include <signal.h>
#include <iostream>


Correlator_node_controller::Correlator_node_controller(Correlator_node &node)
    : Controller(node), node(node) {}

Correlator_node_controller::~Correlator_node_controller() {}

Controller::Process_event_status
Correlator_node_controller::process_event(MPI_Status &status) {
  switch (status.MPI_TAG) {
  case MPI_TAG_DELAY_TABLE: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      MPI_Transfer mpi_transfer;
      Delay_table_akima table;
      int sn;
      mpi_transfer.receive(status, table, sn);
      node.add_delay_table(sn, table);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_UVW_TABLE: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      MPI_Transfer mpi_transfer;
      Uvw_model table;
      int sn;
      mpi_transfer.receive(status, table, sn);
      node.correlation_core.add_uvw_table(sn, table);
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_CORR_PARAMETERS: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      Correlation_parameters parameters;
      MPI_Transfer::receive(status, parameters);

      node.receive_parameters(parameters);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}
