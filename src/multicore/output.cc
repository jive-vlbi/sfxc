/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <types.h>
#include <Output_node.h>

#include <iostream> 
#include <assert.h>

int main(int argc, char *argv[]) {
  // MPI
  int rank;

  //initialisation
  int stat = MPI_Init(&argc,&argv);
  if (stat != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, stat);
    return 1;
  }

  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  
  assert(rank == RANK_OUTPUT_NODE);

  MPI_Status status;
  int32_t msg;
  MPI_Recv(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  assert(status.MPI_TAG == MPI_TAG_SET_OUTPUT_NODE); 

  {
    Output_node output_node(rank);
    output_node.start();
  } // Make sure the destructor is called
  
  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
