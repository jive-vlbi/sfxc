/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: sfxc_mpi.h 281 2007-07-10 13:53:05Z kruithof $
 *
 */

#include <utils.h>
#include <sfxc_mpi.h>
#include <Log_node.h>
#include <Input_node.h>
#include <Output_node.h>
#include <Correlator_node.h>

void start_node() {
  int rank;
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

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
  case MPI_TAG_SET_OUTPUT_NODE: 
    {
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32, 
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      Output_node node(rank);
      node.start();
      break;
    }
  case MPI_TAG_SET_CORRELATOR_NODE: 
    {
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32, 
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      Correlator_node node(rank, msg, /* buffer size */ 10);
      node.start();
      break;
    }
  case MPI_TAG_END_NODE: 
    {
      DEBUG_MSG("MPI_TAG_END_NODE");
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32, 
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      break;
    }
  default:
    {
      std::cout << "Unknown node type " << status.MPI_TAG << std::endl;
      assert(false);
    }
  }
}

void end_node(int32_t rank) {
  MPI_Send(&rank, 1, MPI_INT32, 
           rank, MPI_TAG_END_NODE, MPI_COMM_WORLD);
}
