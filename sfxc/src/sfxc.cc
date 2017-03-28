/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: sfxc.cc 602 2008-03-06 17:12:22Z kruithof $
 *
 *  Tests reading a file from disk and then writing it back using a Data_node
 */


#include <fstream>
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
#include "utils.h"

#include "manager_node.h"

#include "svn_version.h"

#ifdef RUNTIME_STATISTIC
#include "monitor.h"
#endif //RUNTIME_STATISTIC

int main(int argc, char *argv[]) {
  //initialisation
  int provided;
  int stat = MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  if (stat != MPI_SUCCESS) {
    std::cerr << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, stat);
  }

  // MPI
  int numtasks;
  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&RANK_OF_NODE);

  if(provided != MPI_THREAD_MULTIPLE){
    std::cout << RANK_OF_NODE << " : MPI_THREAD_MULTIPLE is not available, got level=" << provided << " instead\n";
  }

  DEBUG_MSG_RANK(0, "svn_version: " << SVN_VERSION);

// Print here compilation option enabled while SFXC is in development
#ifdef PRINT_PROGRESS
  if (RANK_OF_NODE == 0)
    std::cout << "Application compiled with: -DPRINT_PROGRESS" << std::endl;
#endif // PRINT_PROGRESS

#ifdef PRINT_TIMER
  if (RANK_OF_NODE == 0)
    std::cout << "Application compiled with: -DPRINT_TIMER" << std::endl;
#endif // PRINT_TIMER

#ifdef RUNTIME_STATISTIC
  if (RANK_OF_NODE == 0) {
    std::cout << "Application compiled with: -DRUNTIME_STATISTIC" << std::endl;
    std::cout << "Application compiled with: -DRUNTIME_STATISTIC_DIR=" << RUNTIME_STATISTIC_DIR << std::endl;
    if ( !directory_exist(RUNTIME_STATISTIC_DIR) ) {
      create_directory(RUNTIME_STATISTIC_DIR);
    }
  }

#endif // RUNTIME_STATISTIC
#ifdef SFXC_PRINT_DEBUG
  if (RANK_OF_NODE == 0) {
    std::cout << "Application compiled with: -DSFXC_PRINT_DEBUG" << std::endl;
  }
#endif // SFXC_PRINT_DEBUG
#ifdef SFXC_DETERMINISTIC
  if (RANK_OF_NODE == 0)
    std::cout << "Application compiled with: -DSFXC_DETERMINISTIC" << std::endl;
#endif // SFXC_DETERMINISTIC
#ifdef MT_SFXC_ENABLE
  if (RANK_OF_NODE == 0)
    std::cout << "Application compiled with: -DMT_SFXC_ENABLE" << std::endl;
#endif // MT_SFXC
#ifdef DUMMY_CORRELATION
  if (RANK_OF_NODE == 0)
    std::cout << "Application compiled with: -DDUMMY_CORRELATION" << std::endl;
#endif // MT_SFXC
#ifdef MT_MPI_ENABLE
#ifndef MT_SFXC_ENABLE
  std::cout << "Application compiled with: -DMT_MPI_ENABLE but without -DMT_SFXC_ENABLE" << std::endl;
  MPI_Abort(MPI_COMM_WORLD, stat);
#endif
  if (RANK_OF_NODE == 0)
    std::cout << "Application compiled with: -DMT_MPI_ENABLE" << std::endl;
#endif // SFXC_DETERMINISTIC
  // set the hostname
  {
  char hostname[255];
  gethostname(hostname, 255);
  HOSTNAME_OF_NODE = hostname;
  }

  park_miller_set_seed(RANK_OF_NODE+1);

  char *ctrl_file, *vex_file;
  if ( argc == 3 ){
    ctrl_file = argv[1];
    vex_file = argv[2];
  }
  else{
    if ( RANK_OF_NODE == 0 ) {
      std::cerr << "ERROR: invalid number of parameter." << std::endl;
      std::cerr << "usage: sfxc <controlfile> <vexfile>" << std::endl;
    }
    MPI_Abort(MPI_COMM_WORLD, stat);
  }

  if (RANK_OF_NODE == RANK_MANAGER_NODE) {
    Control_parameters control_parameters;

    Log_writer_cout log_writer(10);
    if ((!control_parameters.initialise(ctrl_file, vex_file, std::cout)) ||
        (!control_parameters.check(std::cout))){
      int error = -1;
      MPI_Bcast(&error, 1, MPI_INT32, RANK_MANAGER_NODE, MPI_COMM_WORLD);
      MPI_Barrier( MPI_COMM_WORLD );
      MPI_Finalize();
      exit(1);
    } else {
      Log_writer_mpi log_writer(RANK_OF_NODE, control_parameters.message_level());
      // Determine number of correlator nodes and broadcast to all nodes
      int nr_corr_nodes = numtasks - control_parameters.number_stations() - 3;
      MPI_Bcast(&nr_corr_nodes, 1, MPI_INT32, RANK_MANAGER_NODE, MPI_COMM_WORLD);
      // Create a communicator for all correlator nodes which can be used for 
      // collective communications. Note that ALL mpi processes must create 
      // the communicator not only the correlator nodes.
      create_correlator_node_comm(nr_corr_nodes);

      if (PRINT_PID) {
        DEBUG_MSG("Manager node, pid = " << getpid());
      }
      if (PRINT_HOST) {
        DEBUG_MSG("Manager node, hostname = " << HOSTNAME_OF_NODE);
      }
      ID_OF_NODE = "Managernode";
      LOG_MSG("test");
      Manager_node node(RANK_OF_NODE, numtasks, &log_writer, control_parameters);
      node.start();
    }
  } else {
    int nr_corr_nodes;
    MPI_Bcast(&nr_corr_nodes, 1, MPI_INT32, RANK_MANAGER_NODE, MPI_COMM_WORLD);
    // nr_corr_nodes is negative in case of error
    if (nr_corr_nodes > 0){
      // Create a communicator for all correlator nodes which can be used for 
      // collective communications. Note that ALL mpi processes must create 
      // the communicator not only the correlator nodes.
      create_correlator_node_comm(nr_corr_nodes);
 
      start_node();
    }
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
