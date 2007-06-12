/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Log_node.h>

#include <types.h>
#include <Log_writer.h>
#include <Log_writer_cout.h>
#include <Log_writer_file.h>

#include <iostream>
#include <assert.h>

Log_node::Log_node(int rank, int nNodes) 
  : Node(rank), log_node_ctrl(*this, rank, nNodes)
{
  add_controller(&log_node_ctrl);

  get_log_writer() << "Log_node()" << std::endl;

  INT32 msg;
  MPI_Send(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);
}

void Log_node::start() {
  while (!log_node_ctrl.ready()) {
    check_and_process_message();
  }
}

void Log_node::hook_added_data_reader(size_t reader) {
}
void Log_node::hook_added_data_writer(size_t writer) {
}
