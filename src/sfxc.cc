/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 *  Tests reading a file from disk and then writing it back using a Data_node
 */

#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include "types.h"
#include "input_node.h"
#include "output_node.h"
#include "log_node.h"
#include "log_writer_cout.h"

#include "delay_table_akima.h"
#include "mpi_transfer.h"


#include "node.h"
#include "data_reader2buffer.h"
#include "tcp_connection.h"
#include "buffer2data_writer.h"
#include "data_writer.h"
#include "data_writer_file.h"
#include "data_reader_file.h"
#include "data_reader_tcp.h"
#include "channel_extractor_mark4.h"
#include "utils.h"

#include "manager_node.h"

int main(int argc, char *argv[]) {
  //initialisation
  int stat = MPI_Init(&argc,&argv);
  if (stat != MPI_SUCCESS) {
    std::cerr << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, stat);
  }

  // MPI
  int numtasks, rank;
  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

 // Print here compilation option enabled while SFXC is in development
  #ifdef PRINT_PROGRESS
  if(rank == 0)
    std::cout << "Application compiled with: -DPRINT_PROGRESS" << std::endl;
  #endif // PRINT_PROGRESS

  if( argc != 3 ){
    if( rank == 0 ){
      std::cerr << "ERROR: not enough parameter." << std::endl;
      std::cerr << "usage: sfxc controlfile.ctrl vexfile.vex." << std::endl;
    }
    MPI_Abort(MPI_COMM_WORLD, stat);
  }

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

      if (PRINT_PID) { DEBUG_MSG("Manager node, pid = " << getpid()); }
      if (PRINT_HOST) { 
        char hostname[255]; gethostname(hostname, 255);
        DEBUG_MSG("Manager node, hostname = " << hostname); 
      }
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
