/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "Log_node.h"
#include <assert.h>

#include "Log_writer_cout.h"
#include "Log_writer_file.h"

Log_node_controller::Log_node_controller(Node &node, int rank, int nNodes)
 : Controller(node), 
   log_writer_output(NULL),
   nConnections(nNodes-1)
{
  MPI_Status status, status2;
  MPI_Probe(RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  switch (status.MPI_TAG) {
    case MPI_TAG_LOG_NODE_SET_OUTPUT_COUT: {
      int msg;
      MPI_Recv(&msg, 1, MPI_INT, 
               status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status2);
      set_log_writer(new Log_writer_cout());
      break;
    }
    case MPI_TAG_LOG_NODE_SET_OUTPUT_FILE: {
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size >= 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, 
               status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      set_log_writer(new Log_writer_file(filename));
      break;
    }
    default: {
      // Use the default mpi log writer:
      get_log_writer() << "Unknown log type: " << print_MPI_TAG(status.MPI_TAG) << std::endl; 
    }
  }
  
}

Controller::Process_event_status 
Log_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
    case MPI_TAG_LOG_MESSAGE: {
      assert(log_writer_output != NULL);
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char message[size];
      MPI_Recv(&message, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      (*log_writer_output) << message << std::endl;
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
    case MPI_TAG_LOG_MESSAGES_ENDED: {
      int node;
      MPI_Recv(&node, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      // Use the default mpi log writer:
      get_log_writer() << "  *** Node " << status.MPI_SOURCE
                       << " finished." << std::endl;
      
      nConnections --;
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }

  return PROCESS_EVENT_STATUS_UNKNOWN;
}

void Log_node_controller::set_log_writer(Log_writer *writer) {
  assert(log_writer_output == NULL);
  //if (log_writer != NULL) delete log_writer;
  log_writer_output = writer;
}
