/*
  CVS keywords
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#include <types.h>
#include <Controller_node.h>
#include <Input_node.h>
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
//declarations for offsets
INT64 sliceStartByte[NstationsMax][NprocessesMax];
INT64 sliceStopByte [NstationsMax][NprocessesMax];
INT64 sliceStartTime [NprocessesMax];
INT64 sliceStopTime  [NprocessesMax];
INT64 sliceTime;

#include <iostream> 
#include <assert.h>



int main(int argc, char *argv[])
{
  // MPI
  int numtasks, rank;

  //initialisation
  int status = MPI_Init(&argc,&argv);
  if (status != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, status);
  }

  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  if (argc != 2) {
    if (rank == 0) {
      std::cout << "Usage: " << argv[0] << " <ctrl-file>" << std::endl;
    }
    
	//close the mpi stuff
  	MPI_Finalize();
    
    exit(0);
  }

  if (rank == 0) {
    Controller_node controller(numtasks, rank, argv[1]);
    controller.start();
  } else {
    MPI_Status status, status2;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    switch (status.MPI_TAG) {
    case MPI_TAG_SET_INPUT_NODE_FILE: 
      {
        assert(status.MPI_SOURCE == 0);
        int size;
        MPI_Get_elements(&status, MPI_CHAR, &size);
        char filename[size];
        MPI_Recv(&filename, size, MPI_CHAR, 0, 
                 MPI_ANY_TAG, MPI_COMM_WORLD, &status2);

        assert(status.MPI_SOURCE == status2.MPI_SOURCE);
        assert(status.MPI_TAG == status2.MPI_TAG);

        Data_node data_node(rank, filename);
        Data_node.start();
        break;
      }
    case MPI_TAG_SET_CORRELATOR_NODE: 
      {
        int job;
        MPI_Recv(&job, 1, MPI_INT, 0, 
                 MPI_ANY_TAG, MPI_COMM_WORLD, &status2);
        assert(status.MPI_SOURCE == status2.MPI_SOURCE);
        assert(status.MPI_TAG == status2.MPI_TAG);

        Correlator_node correlator(rank);
        correlator.start();
      break;
    }
    default:
      std::cout << "Unknown node type" << std::endl;
      assert(false);
      return 1;
    }
  }

  //close the mpi stuff
  MPI_Finalize();

  return 1;

}

