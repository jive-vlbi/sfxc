#include <Input_node.h>
#include <iostream>

#include <types.h>
#include <assert.h>

Input_node::Input_node(int rank) : Node(rank) {
  write_debug("Input_node: constructor");
  char input_name[256];
  MPI_Status status;
  MPI_Recv(&input_name, 256, MPI_CHAR, 0, 
           MPI_TAG_INIT, MPI_COMM_WORLD, &status);
  write_debug(input_name);
}

void Input_node::start() {
  while (true) {
    int command;
    MPI_Status status;
    MPI_Recv(&command, 1, MPI_INT, 0, 
             MPI_TAG_COMMUNICATION, MPI_COMM_WORLD, &status);
    switch (command) {
    case MPI_CORRELATION_READY: 
      {
        write_debug("Correlation finished, stopping.");
        return;
      }
    default: 
      {
        write_debug("unknown tag, quitting.");
        assert(false);
        return;
      }
    }
  }
}
