/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <types.h>
#include <Manager_node.h>
#include <Input_node.h>
#include <Output_node.h>
#include <Correlator_node.h>
#include <Log_node.h>

#include <Control_parameters.h>

#include <iostream> 
#include <assert.h>


int main(int argc, char *argv[]) {
  // MPI
  int rank;

  //initialisation
  int status = MPI_Init(&argc,&argv);
  if (status != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, status);
    return 1;
  }

  assert(argc == 3);
  char *vex_file = argv[1];
  char *ctrl_file = argv[2];

  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  ///////////////////////////
  //  The real work
  ///////////////////////////
  assert (rank == RANK_MANAGER_NODE);

  // get the number of tasks set at commandline (= number of processors)
  int numtasks;
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

  {  
    Log_writer_mpi log_writer(rank);
    Control_parameters control_parameters;
    control_parameters.initialise(vex_file, ctrl_file, log_writer);
    
    Manager_node manager(rank, numtasks, &log_writer, control_parameters);
    manager.start();
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
