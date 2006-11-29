#include <Input_node.h>

#include <types.h>
#include <Input_reader_file.h>

#include <iostream>
#include <assert.h>

Input_node::Input_node(int rank) : Node(rank) {
  write_debug("Input_node: constructor");
  MPI_Status status, status2;
  MPI_Probe(0, MPI_TAG_INIT, MPI_COMM_WORLD, &status);
  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  char input[size];
  MPI_Recv(input, size, MPI_CHAR, 0, 
           MPI_TAG_INIT, MPI_COMM_WORLD, &status2);
  assert(status.MPI_SOURCE == status2.MPI_SOURCE);
  assert(status.MPI_TAG == status2.MPI_TAG);
  write_debug(input);
  reader = new Input_reader_file(input);
}

Input_node::~Input_node() {
  delete reader;
}

void Input_node::start() {
  while (true) {
    MPI_Status status, status2;
    MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    switch (status.MPI_TAG) {
    case MPI_TAG_COMMUNICATION: 
      {
        int command;
        MPI_Recv(&command, 1, MPI_INT, status.MPI_SOURCE, 
                 MPI_TAG_COMMUNICATION, MPI_COMM_WORLD, &status2);
        assert(status.MPI_SOURCE == status2.MPI_SOURCE);
        assert(status.MPI_TAG == status2.MPI_TAG);
        
        switch (command) {
        case MPI_CORRELATION_READY: {
          write_debug("Correlation finished, stopping Input_node.");
          return;
        }
        default: {
          assert(false);
          return;
        }
        }
        break;
      }
    default: 
      {
        char msg[25];
        sprintf(msg, "Unknown command %d", status.MPI_TAG);
        write_debug(msg);
        assert(false);
        return;
      }
    }
  }
}
