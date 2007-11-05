/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 *  Tests reading a file from disk and then writing it back using a Data_node
 */

#include <types.h>
#include <Input_node.h>
#include <Output_node.h>
#include <Log_node.h>
#include <Log_writer_cout.h>

#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include "Delay_table_akima.h"
#include "MPI_Transfer.h"


#include <Node.h>
#include <Data_reader2buffer.h>
#include <TCP_Connection.h>
#include <Buffer2data_writer.h>
#include <Data_writer.h>
#include <Data_writer_file.h>
#include <Data_reader_file.h>
#include <Data_reader_tcp.h>
#include <Channel_extractor_mark4.h>
#include <utils.h>

#include <Manager_node.h>

int main(int argc, char *argv[]) {
  //initialisation
  int stat = MPI_Init(&argc,&argv);
  if (stat != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, stat);
  }

  // MPI
  int numtasks, rank;
  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  DEBUG_MSG(" pid = " << getpid());

  assert(argc == 3);
  char *ctrl_file = argv[1];
  char *vex_file = argv[2];

  if (rank == RANK_MANAGER_NODE) {
    Control_parameters control_parameters;

    Log_writer_cout log_writer(10);
    control_parameters.initialise(ctrl_file, vex_file, log_writer);
    if (!control_parameters.check(std::cout)) {
      for (int i=0; i<numtasks; i++) {
        if (i != RANK_MANAGER_NODE) {
          end_node(i);
        }
      }
    } else {
      Log_writer_mpi log_writer(rank, control_parameters.message_level());
      Manager_node node(rank, numtasks, &log_writer, control_parameters);
      node.start();
    }
  } else {
    start_node();
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
