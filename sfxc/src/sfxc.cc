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


#include <iostream> 
#include <assert.h>

#include <genFunctions.h>


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

  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  //std::cout << "#" << rank << " pid = " << getpid() << std::endl;

  ///////////////////////////
  //  The real work
  ///////////////////////////
  if (rank == RANK_MANAGER_NODE) {
    // get the number of tasks set at commandline (= number of processors)
    int numtasks;
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    
    Manager_node manager(numtasks, rank, argv[1]);
    manager.start();
  } else {
    MPI_Status status;
    MPI_Probe(RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    switch (status.MPI_TAG) {
    case MPI_TAG_SET_LOG_NODE:
      { 
        int32_t msg;
        MPI_Recv(&msg, 1, MPI_INT32, 
                 RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        // No break
      }
    case MPI_TAG_LOG_MESSAGE: 
      {
        assert (RANK_LOG_NODE == rank);
        int numtasks;
        MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
        Log_node log_node(rank,numtasks);
        log_node.start();
        break;
      }
    case MPI_TAG_SET_INPUT_NODE: 
      {
        // The integer is the number of the input_reader:
        int32_t msg;
        MPI_Recv(&msg, 1, MPI_INT32, 
                 RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        Input_node input_node(rank, msg);
        input_node.start();
        break;
      }
    case MPI_TAG_SET_CORRELATOR_NODE: 
      {
        int32_t msg;
        MPI_Recv(&msg, 1, MPI_INT32, 
                 RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        Correlator_node correlator(rank, msg, 10);
        correlator.start();
        break;
      }
    case MPI_TAG_SET_OUTPUT_NODE: 
      {
        int32_t msg;
        MPI_Recv(&msg, 1, MPI_INT32, 
                 RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        Output_node node(rank);
        node.start();
        break;
      }
    default:
      {
        std::cout << "Unknown node type " << status.MPI_TAG << std::endl;
        assert(false);
        return 1;
      }
    }
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
