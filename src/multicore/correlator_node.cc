/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: sfxc_adam.cc 174 2007-03-20 09:08:42Z kruithof $
 */

#include <types.h>
#include <Correlator_node.h>

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
UINT32 seed;


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
  INT32 msg;
  MPI_Recv(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  assert(status.MPI_TAG == MPI_TAG_SET_CORRELATOR_NODE); 

  {
    Correlator_node correlator_node(rank);
    correlator_node.start();
  } // Make sure the destructor is called
  
  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
