/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: sfxc_mpi.h 281 2007-07-10 13:53:05Z kruithof $
 *
 */


#include "sfxc_mpi.h"
#include "utils.h"
#include "log_node.h"
#include "input_node.h"
#include "output_node.h"
#include "correlator_node.h"

IF_MT_MPI_ENABLED( Mutex g_mpi_thebig_mutex );

void start_node(int swap) {
  int rank;
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  MPI_Status status;
  MPI_Probe(RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_LOG_NODE: {
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32,
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      // No break
    }
  case MPI_TAG_LOG_MESSAGE: {

      SFXC_ASSERT (RANK_LOG_NODE == rank);
      int numtasks;
      MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
      if (PRINT_PID) {
        DEBUG_MSG("Log node, pid = " << getpid());
      }
      if (PRINT_HOST) {
        char hostname[255];
        gethostname(hostname, 255);
        DEBUG_MSG("Log node, hostname = " << hostname);
      }

      Log_node log_node(rank,numtasks);
      log_node.start();
      break;
    }
  case MPI_TAG_SET_INPUT_NODE_MARK5A: {
      // The integer is the number of the input_reader:
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32,
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      if (PRINT_PID) {
        DEBUG_MSG("Input node, pid = " << getpid());
      }
      if (PRINT_HOST) {
        char hostname[255];
        gethostname(hostname, 255);
        DEBUG_MSG("Input node, hostname = " << hostname);
      }
      Input_node input_node(rank, msg, MARK5A);
      input_node.start();
      break;
    }
  case MPI_TAG_SET_INPUT_NODE_VLBA: {
      // The integer is the number of the input_reader:
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32,
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      if (PRINT_PID) {
        DEBUG_MSG("Input node, pid = " << getpid());
      }
      if (PRINT_HOST) {
        char hostname[255];
        gethostname(hostname, 255);
        DEBUG_MSG("Input node, hostname = " << hostname);
      }
      Input_node input_node(rank, msg, VLBA);
      input_node.start();
      break;
    }
  case MPI_TAG_SET_INPUT_NODE_MARK5B: {
      // The integer is the number of the input_reader:
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32,
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      if (PRINT_PID) {
        DEBUG_MSG("Input node, pid = " << getpid());
      }
      if (PRINT_HOST) {
        char hostname[255];
        gethostname(hostname, 255);
        DEBUG_MSG("Input node, hostname = " << hostname);
      }
      Input_node input_node(rank, msg, MARK5B);
      input_node.start();
      break;
    }
  case MPI_TAG_SET_OUTPUT_NODE: {
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32,
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      if (PRINT_PID) {
        DEBUG_MSG("Output node, pid = " << getpid());
      }
      if (PRINT_HOST) {
        char hostname[255];
        gethostname(hostname, 255);
        DEBUG_MSG("Output node, hostname = " << hostname);
      }
      Output_node node(rank);
      node.start();
      break;
    }
  case MPI_TAG_SET_CORRELATOR_NODE: {
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32,
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      if (PRINT_PID) {
        DEBUG_MSG("Correlator node, pid = " << getpid());
      }
      if (PRINT_HOST) {
        char hostname[255];
        gethostname(hostname, 255);
        DEBUG_MSG("Correlator node, hostname = " << hostname);
      }
      Correlator_node node(rank, msg, swap);
      node.start();
      break;
    }
  case MPI_TAG_END_NODE: {
      DEBUG_MSG("MPI_TAG_END_NODE");
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32,
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      break;
    }
  default: {
      SFXC_ASSERT_MSG(false, "Unknown node type");
    }
  }
}

void end_node(int32_t rank) {
  MPI_Send(&rank, 1, MPI_INT32,
           rank, MPI_TAG_END_NODE, MPI_COMM_WORLD);
}
