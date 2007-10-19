/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Input_node.h>

#include <types.h>
#include <Semaphore_buffer.h>

#include <Data_reader_buffer.h>
#include <Channel_extractor_mark4.h>

#include <iostream>
#include <assert.h>
#include <time.h>
#include <math.h>

#include <MPI_Transfer.h>

#include "input_reader.h"
#include "output_writer.h"
#include "tasklet/singleton.h"

Input_node::Input_node(int rank, int station_number, Log_writer *log_writer) 
  : Node(rank, log_writer)
{
  initialise();
}
Input_node::Input_node(int rank, int station_number) 
  : Node(rank)
{
  initialise();
}

void Input_node::initialise()  {
  get_log_writer() << "Input_node()" << std::endl;

  TaskletPool &pool = tasklet_manager.get_pool();
  
  Input_reader input_reader(pool, 
                            "file://./input_reader.o");

  Output_writer output_writer(pool, 
                              "file://./output.txt");

  // connect the buffers
  output_writer.connect_to(input_reader.get_output_buffer());

  // start the pools
  input_reader.start();
  output_writer.start();

  singleton<ThreadPool>::instance().wait_for_all_termination();

//   int32_t msg;
//   MPI_Send(&msg, 1, MPI_INT32, 
//            RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);
}

Input_node::~Input_node() {
}
