/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: test_Input_node.cc 273 2007-06-28 13:54:22Z kruithof $
 *
 *  Tests reading a file from disk and then writing it back using a Data_node
 */

#include <types.h>
#include <Output_node.h>
#include <Log_node.h>
#include <Log_writer_cout.h>

#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include "constPrms.h"
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "InData.h"
#include "delayTable.h"
#include "MPI_Transfer.h"


#include <Node.h>
#include <Data_reader2buffer.h>
#include <TCP_Connection.h>
#include <Buffer2data_writer.h>
#include <Data_writer.h>
#include <Data_writer_file.h>
#include <Data_writer_void.h>
#include <Data_reader_tcp.h>
#include <utils.h>

const int output_node = 2;
const char output_file_to_disk[] = "output_file_to_disk.dat";


int64_t start_time = -1;
int64_t stop_time  = -1;


void wait_for_setting_up_channel(int rank) {
  MPI_Status status;
  int64_t channel;
  if (rank >= 0) {
    MPI_Recv(&channel, 1, MPI_INT64, rank,
             MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
  } else {
    MPI_Recv(&channel, 1, MPI_INT64, MPI_ANY_SOURCE,
             MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
  }
}

void test_output_node(int rank, int numtasks,
                      const char *input_file,
                      char *output_file) {
  MPI_Status status;
  
  if (rank == RANK_MANAGER_NODE) {
    int32_t msg=0;
    // Log node:
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_SET_LOG_NODE, MPI_COMM_WORLD);
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_LOG_NODE_SET_OUTPUT_COUT, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);

    // Initialise control parameters
    Log_writer_mpi log_writer(rank);

    // Output_node node:
    msg = 0;
    MPI_Send(&msg, 1, MPI_INT32, 
             output_node, MPI_TAG_SET_OUTPUT_NODE, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             output_node, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);

    // Input data:
    int reader_stream_nr = 3;
    char infile_msg[strlen(input_file)+1];
    sprintf(infile_msg, "%c%s", (char)reader_stream_nr, input_file);
    MPI_Send(infile_msg, strlen(infile_msg)+1, 
             MPI_CHAR, 
             output_node, MPI_TAG_ADD_DATA_READER_FILE, MPI_COMM_WORLD);
    wait_for_setting_up_channel(output_node);

    MPI_Send(output_file, strlen(output_file)+1, MPI_CHAR, 
             output_node, MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);
    wait_for_setting_up_channel(output_node);

    int slicenr = 0;
    {
      int nBytes = 1000;
      int64_t priority[] = {reader_stream_nr, slicenr, nBytes};
      MPI_Send(&priority, 3, MPI_INT64, 
               output_node, MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY, 
               MPI_COMM_WORLD);
      slicenr++;
    }
    MPI_Send(&slicenr, 1, MPI_INT32, output_node, 
             MPI_TAG_OUTPUT_NODE_CORRELATION_READY, MPI_COMM_WORLD);

    MPI_Send(&rank, 1, MPI_INT, 
             RANK_LOG_NODE, MPI_TAG_LOG_MESSAGES_ENDED, MPI_COMM_WORLD);
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
        {
          assert (RANK_LOG_NODE == rank);
          Log_node log_node(rank,numtasks);
          log_node.start();
        }
        {
          int32_t msg;
          MPI_Recv(&msg, 1, MPI_INT32, 
                   rank, MPI_TAG_LOG_MESSAGES_ENDED, 
                   MPI_COMM_WORLD, &status);
        }
        break;
      }
    case MPI_TAG_SET_OUTPUT_NODE: 
      {
        int32_t msg;
        MPI_Recv(&msg, 1, MPI_INT32, 
                 RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        assert(rank == output_node);
        Output_node node(rank);
        node.start();
        break;
      }
    default: 
      {
        std::cout << "Received a " <<print_MPI_TAG(status.MPI_TAG) << " tag." << std::endl;
        assert(false);
        break;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  assert(output_node+RANK_MANAGER_NODE+RANK_LOG_NODE == 3);

  //initialisation
  int stat = MPI_Init(&argc,&argv);
  if (stat != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, stat);
  }

  // MPI
  int numtasks, rank;
  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  assert(numtasks == 3);
  
  DEBUG_MSG(" pid = " << getpid());

  assert(argc == 3);
  const char *control_file = argv[1];
  const char *output_directory = argv[2];
  
  // First test
  int len = strlen(output_directory)+strlen(output_file_to_disk)+4;
  char output_file1[len];
  char output_file2[len];
  snprintf(output_file1, len, "%s/%s.1", 
           output_directory, output_file_to_disk);
  snprintf(output_file2, len, "%s/%s.2", 
           output_directory, output_file_to_disk);

  {
    // 0: controller
    // 1: log node
    // 2: output node
    
    test_output_node(rank, numtasks, control_file, output_file1);
    MPI_Barrier( MPI_COMM_WORLD );
    test_output_node(rank, numtasks, control_file, output_file2);
    MPI_Barrier( MPI_COMM_WORLD );
  
    if (rank == output_node) {
      sync();
      std::stringstream cmd; 
      cmd << "cmp " << output_file1 << " " << output_file2;
      if (system(cmd.str().c_str())) {
        std::cout << "cmp file output failed" << std::endl;
        return 1;
      }
    }
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
