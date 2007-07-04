/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <types.h>
#include <Input_node.h>

//global variables
#include <runPrms.h>
#include <genPrms.h>
#include <staPrms.h>
#include <constPrms.h>
//declaration and default settings run parameters
RunP RunPrms;
//declaration and default settings general parameters
GenP GenPrms;
//station parameters class, declaration and default settings
StaP StaPrms[NstationsMax];
// used for randomising numbers for Headers in Mk4 file
uint32_t seed;


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

  MPI_Status status;
  int32_t msg;
  MPI_Recv(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  assert(status.MPI_TAG == MPI_TAG_SET_INPUT_NODE); 

  {
    // msg is the number of the input node
    Input_node input_node(rank);
    input_node.start();
  } // Make sure the destructor is called

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
