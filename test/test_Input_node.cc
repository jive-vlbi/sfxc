/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 *  Tests reading a file from disk and then writing it back using a Data_node
 */

#include <types.h>
#include <Input_node.h>
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
#include <Data_reader_file.h>
#include <Data_reader_tcp.h>
#include <Channel_extractor_mark4.h>
#include <utils.h>

RunP RunPrms;
GenP GenPrms;
StaP StaPrms[NstationsMax];

const int input_node = 2;
const int output_node = 3;
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

void initialise(const char *control_file) {
  Log_writer_cout log_writer;
  initialise_control(control_file, log_writer, RunPrms, GenPrms, StaPrms);

  start_time = GenPrms.get_usStart();
  stop_time   = start_time + GenPrms.get_usDur()+1000000;
}

void test_writing_data_to_file(int rank, int numtasks,
                               const char *control_file,
                               const char *output_file) {
  MPI_Status status;
  
  if (rank == RANK_MANAGER_NODE) {
    int msg=0;
    // Log node:
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_SET_LOG_NODE, MPI_COMM_WORLD);
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_LOG_NODE_SET_OUTPUT_COUT, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);

    // Initialise control parameters
    Log_writer_mpi log_writer(rank);

    // Input_node node: integer is the station number
    msg = 0;
    MPI_Send(&msg, 1, MPI_INT32, 
             input_node, MPI_TAG_SET_INPUT_NODE, MPI_COMM_WORLD);

    MPI_Transfer mpi_transfer;
    mpi_transfer.send_general_parameters(input_node, RunPrms, GenPrms, StaPrms);

    MPI_Recv(&msg, 1, MPI_INT32, 
             input_node, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
        
    // Connect the input and output:
    // strlen+1 so that \0 gets transmitted as well
    MPI_Send(StaPrms[0].get_mk4file(), strlen(StaPrms[0].get_mk4file())+1, 
             MPI_CHAR, 
             input_node, MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);
    wait_for_setting_up_channel(input_node);
    
    { // send output file to input node
      int size = strlen(output_file)+2;
      char buffer[size];
      buffer[0] = (char)output_node;
      strncpy(buffer+1, output_file, size-1);
    
      MPI_Send(buffer, size, MPI_CHAR, 
               input_node, MPI_TAG_ADD_DATA_WRITER_FILE, MPI_COMM_WORLD);
      wait_for_setting_up_channel(input_node);
    }

    {
      int64_t start_time_check;
      // Get initial start time
      MPI_Send(&start_time_check, 1, MPI_INT64, 
               input_node, MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP, 
               MPI_COMM_WORLD);
      MPI_Recv(&start_time_check, 1, MPI_INT64, 
               input_node, MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP, 
               MPI_COMM_WORLD, &status);
      assert(start_time_check <= start_time);

      // Set the right start time:
      MPI_Send(&start_time, 1, MPI_INT64, 
               input_node, MPI_TAG_INPUT_NODE_GOTO_TIME, MPI_COMM_WORLD);

      // Check start time:
      MPI_Send(&start_time_check, 1, MPI_INT64, 
               input_node, MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP, 
               MPI_COMM_WORLD);
      MPI_Recv(&start_time_check, 1, MPI_INT64, 
               input_node, MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP, 
               MPI_COMM_WORLD, &status);
      assert(start_time_check == start_time);
      
    }

    int64_t priority_in[] = {output_node, 0, start_time, stop_time};
    MPI_Send(&priority_in, 4, MPI_INT64, 
             input_node, MPI_TAG_INPUT_NODE_INPUT_STREAM_SET_PRIORITY, MPI_COMM_WORLD);

    MPI_Send(&stop_time, 1, MPI_INT64, 
             input_node, MPI_TAG_INPUT_NODE_STOP_TIME, MPI_COMM_WORLD);

    // Wait for data nodes to finish
    MPI_Status status;
    int i;
    MPI_Recv(&i, 1, MPI_INT32, MPI_ANY_SOURCE,
             MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD, &status);
    assert(status.MPI_TAG == MPI_TAG_DATASTREAM_EMPTY);
    assert(status.MPI_SOURCE == input_node);
   
    MPI_Send(&rank, 1, MPI_INT, 
             RANK_LOG_NODE, MPI_TAG_LOG_MESSAGES_ENDED, MPI_COMM_WORLD);

  } else {
    // Don't use the output node:
    if (rank == output_node) {
      return;
    }
    
    int32_t msg;
    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    switch (status.MPI_TAG) {
    case MPI_TAG_SET_LOG_NODE: {
      assert (RANK_LOG_NODE == rank);
      {
        // The output node does not participate
        Log_node log_node(rank,numtasks-1);
        log_node.start();
      }
      { // Receive the message of the log node itself
        MPI_Recv(&msg, 1, MPI_INT32, 
                 rank, MPI_TAG_LOG_MESSAGES_ENDED, 
                 MPI_COMM_WORLD, &status);
      }
      break;
    }
    case MPI_TAG_SET_INPUT_NODE: {
      assert(rank == input_node);
      // msg is the station number
      Input_node node(rank, msg);
      node.start();
      break;
    }
    default: {
      assert(false);
      break;
    }
    }
  }
}

void test_writing_data_to_output_node_using_TCP(int rank, int numtasks,
                                                const char *control_file,
                                                const char *output_file) {
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

    // Input_node node: msg is the station number
    msg = 0;
    MPI_Send(&msg, 1, MPI_INT32, 
             input_node, MPI_TAG_SET_INPUT_NODE, MPI_COMM_WORLD);
    MPI_Transfer mpi_transfer;
    mpi_transfer.send_general_parameters(input_node, RunPrms,GenPrms,StaPrms);
    MPI_Recv(&msg, 1, MPI_INT32, 
             input_node, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);

    // Output_node node:
    msg = 0;
    MPI_Send(&msg, 1, MPI_INT32, 
             output_node, MPI_TAG_SET_OUTPUT_NODE, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             output_node, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);

    char out_file[strlen(output_file)+1];
    strncpy(out_file, output_file, strlen(output_file)+1);
    MPI_Send(out_file, strlen(out_file)+1, MPI_CHAR, 
             output_node, MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);
    wait_for_setting_up_channel(output_node);

    


    char *filename = StaPrms[0].get_mk4file();
    MPI_Send(filename, strlen(filename)+1, 
             MPI_CHAR, 
             input_node, MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);

    wait_for_setting_up_channel(input_node);

    // Set the right start time:
    MPI_Send(&start_time, 1, MPI_INT64, 
             input_node, MPI_TAG_INPUT_NODE_GOTO_TIME, MPI_COMM_WORLD);

    // Connect input and output
    // the numbers are arbitrary
    int writer_stream_nr = 7;
    int reader_stream_nr = 5;
    int32_t ranks[] = { writer_stream_nr, reader_stream_nr, output_node};
    MPI_Send(ranks, 3, MPI_INT32, 
             input_node,
             MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP, 
             MPI_COMM_WORLD);

    wait_for_setting_up_channel(input_node);

    assert(start_time > 0); 
    assert(stop_time > 0);

    // set priorities:
    {
      int slice_nr = 0;
      int64_t priority_in[] = {writer_stream_nr, slice_nr, start_time, stop_time};
      MPI_Send(&priority_in, 4, MPI_INT64, 
               input_node, MPI_TAG_INPUT_NODE_INPUT_STREAM_SET_PRIORITY, MPI_COMM_WORLD);
  
      MPI_Send(&stop_time, 1, MPI_INT64, 
               input_node, MPI_TAG_INPUT_NODE_STOP_TIME, MPI_COMM_WORLD);

      // Data is not written to file but sent to another node.
      int64_t msg_output_node[] =
        {reader_stream_nr, 
         slice_nr,
         ((stop_time-start_time) * 
          (StaPrms[0].get_tbr() * StaPrms[0].get_fo()*2 * 1024000)) / 8000000};
      MPI_Send(&msg_output_node, 3, MPI_INT64,
               output_node,
               MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY,
               MPI_COMM_WORLD);

    }

    // Wait for data nodes to finish
    MPI_Status status;
    int i;
    MPI_Recv(&i, 1, MPI_INT32, MPI_ANY_SOURCE,
             MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD, &status);
    assert(status.MPI_TAG == MPI_TAG_DATASTREAM_EMPTY);
    assert(status.MPI_SOURCE == input_node);
   
    int32_t slicenr = 1;
    MPI_Send(&slicenr, 1, MPI_INT32, output_node, 
             MPI_TAG_OUTPUT_NODE_CORRELATION_READY, MPI_COMM_WORLD);
//     MPI_Send(&slicenr, 1, MPI_INT32, output_node, 
//              MPI_TAG_END_NODE, MPI_COMM_WORLD);

    MPI_Send(&rank, 1, MPI_INT, 
             RANK_LOG_NODE, MPI_TAG_LOG_MESSAGES_ENDED, MPI_COMM_WORLD);

  } else {
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
        {
          assert (RANK_LOG_NODE == rank);
          Log_node log_node(rank,numtasks);
          log_node.start();
        }
        { // Receive the message of the log node itself
          int32_t msg;
          MPI_Recv(&msg, 1, MPI_INT32, 
                   rank, MPI_TAG_LOG_MESSAGES_ENDED, 
                   MPI_COMM_WORLD, &status);
        }
        break;
      }
    case MPI_TAG_SET_INPUT_NODE: {
      // The integer is the number of the input_reader:
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32, 
               RANK_MANAGER_NODE, MPI_TAG_SET_INPUT_NODE, MPI_COMM_WORLD, &status);
      assert(rank == input_node);
      // msg is the station number
      Input_node node(rank,msg);
      node.start();
      break;
    }
    case MPI_TAG_SET_OUTPUT_NODE: {
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32, 
               RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      assert(rank == output_node);
      Output_node node(rank);
      node.start();
      break;
    }
    default: {
      std::cout << "Received a " <<print_MPI_TAG(status.MPI_TAG) << " tag." << std::endl;
      assert(false);
      break;
    }
    }
  }
}

int main(int argc, char *argv[]) {
  assert(input_node+output_node+RANK_MANAGER_NODE+RANK_LOG_NODE == 6);

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
  assert(numtasks == 4);
  
  assert(argc == 3);
  char control_file[80], output_directory[80];
  strcpy(control_file, argv[1]);
  strcpy(output_directory, argv[2]);
  
  // First test
  int len = strlen(output_directory)+strlen(output_file_to_disk)+4;
  char output_file1[len], output_file2[len];
  snprintf(output_file1, len, "%s/%s.1", 
           output_directory, output_file_to_disk);
  snprintf(output_file2, len, "%s/%s.2", 
           output_directory, output_file_to_disk);

  if (rank == 0) initialise(control_file);

  if (true) {
    // 0: controller
    // 1: log node
    // 2: input node
    // 3: output node
    
    test_writing_data_to_file(rank, numtasks, control_file, output_file1);
    MPI_Barrier( MPI_COMM_WORLD );
    test_writing_data_to_file(rank, numtasks, control_file, output_file2);
    MPI_Barrier( MPI_COMM_WORLD );
  
    if (rank == input_node) {
      sync();
      std::stringstream cmd; 
      cmd << "cmp " << output_file1 << " " << output_file2;
      if (system(cmd.str().c_str())) {
        std::cout << "cmp file output failed" << std::endl;
        return 1;
      }
    }
  }    

  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Status status;
  int result;
  MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &result, &status);
  assert(!result);
  MPI_Barrier( MPI_COMM_WORLD );

  {
    snprintf(output_file2, len, "%s/%s.3", 
             output_directory, output_file_to_disk);
    test_writing_data_to_output_node_using_TCP(rank, numtasks, 
                                               control_file, output_file2);
    MPI_Barrier( MPI_COMM_WORLD );
  
    if (rank == output_node) {
      sync();
      std::stringstream cmd; cmd << "cmp " << output_file1 << " " << output_file2;
      if (system(cmd.str().c_str())) {
        std::cout << "cmp tcp output failed" << std::endl;
        return 1;
      }
    }
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
