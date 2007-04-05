/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <iostream>
#include <assert.h>

//#include "Node.h"
#include "Log_node.h"
//#include "Log_writer_cout.h"
#include "Log_writer_mpi.h"
//#include "Log_controller.h"

//class Test_log_node : public Node {
//public:
//  Test_log_node(int rank) : Node(rank), log_controller(log_writer) {
//    add_controller(&log_controller);
//  }
//private:
//  Log_writer_cout log_writer;
//  Log_controller log_controller;
//};

int main(int argc, char *argv[]) {
  // MPI
  int numtasks, rank;

  //initialisation
  int status = MPI_Init(&argc,&argv);
  if (status != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, status);
    return 1;
  }

  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  assert(numtasks==2);
  
  if (rank==0) {
    int i=0;

    MPI_Send(&i, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_LOG_NODE_SET_OUTPUT_COUT, MPI_COMM_WORLD);

    Log_writer_mpi writer(rank);
    writer << "a\nb\nc\n";
    MPI_Send(&i, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_LOG_MESSAGES_ENDED, MPI_COMM_WORLD);
  } else {
    assert (rank == RANK_LOG_NODE);
    Log_node node(rank, numtasks);
    node.start();
  }


  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
