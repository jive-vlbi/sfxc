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
#include <sfxc_mpi.h>
#include <iostream>
#include <assert.h>
#include <limits>


int main(int argc, char *argv[]) {
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
  assert(numtasks == 2);
  
  int8_t int8(std::numeric_limits<int8_t>::max());
  uint8_t uint8(std::numeric_limits<uint8_t>::max());
  int16_t int16(std::numeric_limits<int16_t>::max());
  uint16_t uint16(std::numeric_limits<uint16_t>::max());
  int32_t int32(std::numeric_limits<int32_t>::max());
  uint32_t uint32(std::numeric_limits<uint32_t>::max());
  int64_t int64(std::numeric_limits<int64_t>::max());

  if (rank == 0) {
//     std::cout << (int)int8 << "\t\t\t" << (int)uint8 << std::endl
//               << int16 << "\t\t\t" << uint16 << std::endl
//               << int32 << "\t\t" << uint32 << std::endl
//               << int64 << std::endl;
  
    int dest = 1;
    MPI_Send(&int8, 1, MPI_INT8, dest, 8, MPI_COMM_WORLD);
    MPI_Send(&uint8, 1, MPI_UINT8, dest, 9, MPI_COMM_WORLD);
    MPI_Send(&int16, 1, MPI_INT16, dest, 16, MPI_COMM_WORLD);
    MPI_Send(&uint16, 1, MPI_UINT16, dest, 17, MPI_COMM_WORLD);
    MPI_Send(&int32, 1, MPI_INT32, dest, 32, MPI_COMM_WORLD);
    MPI_Send(&uint32, 1, MPI_UINT32, dest, 33, MPI_COMM_WORLD);
    MPI_Send(&int64, 1, MPI_INT64, dest, 64, MPI_COMM_WORLD);
  } else {
    int8_t int8_; uint8_t uint8_; 
    int16_t int16_; uint16_t uint16_;
    int32_t int32_; uint32_t uint32_; 
    int64_t int64_; 

    int source = 0;
    MPI_Status status;
    MPI_Recv(&int8_, 1, MPI_INT8, source, 8, MPI_COMM_WORLD, &status);
    assert(int8 == int8_);
    MPI_Recv(&uint8_, 1, MPI_UINT8, source, 9, MPI_COMM_WORLD, &status);
    assert(uint8 == uint8_);

    MPI_Recv(&int16_, 1, MPI_INT16, source, 16, MPI_COMM_WORLD, &status);
    assert(int16 == int16_);
    MPI_Recv(&uint16_, 1, MPI_UINT16, source, 17, MPI_COMM_WORLD, &status);
    assert(uint16 == uint16_);

    MPI_Recv(&int32_, 1, MPI_INT32, source, 32, MPI_COMM_WORLD, &status);
    assert(int32 == int32_);
    MPI_Recv(&uint32_, 1, MPI_UINT32, source, 33, MPI_COMM_WORLD, &status);
    assert(uint32 == uint32_);

    MPI_Recv(&int64_, 1, MPI_INT64, source, 64, MPI_COMM_WORLD, &status);
    assert(int64 == int64_);
//     MPI_Recv(&uint64_, 1, MPI_UINT64, source, 65, MPI_COMM_WORLD, &status);
//     assert(int64 == int64_);
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
