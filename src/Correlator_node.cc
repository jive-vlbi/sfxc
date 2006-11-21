#include <Correlator_node.h>
#include <types.h>
#include <assert.h>

Correlator_node::Correlator_node(int rank) : Node(rank) {
  write_debug("Correlator_node: constructor");
  int type = MPI_CORRELATOR_READY;
  MPI_Send(&type, 1, MPI_INT, 0, MPI_TAG_INIT, MPI_COMM_WORLD);
}

void Correlator_node::start() {
  while (true) {
    MPI_Status status, status2;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    switch (status.MPI_TAG) {
    case MPI_TAG_ADD_INPUT_MPI: 
      {
        int command;
        MPI_Recv(&command, 1, MPI_INT, status.MPI_SOURCE, 
             MPI_TAG_ADD_INPUT_MPI, MPI_COMM_WORLD, &status2);
        assert(status.MPI_TAG == status2.MPI_TAG);
        char msg[25];
        sprintf(msg, "adding input channel %d", command);
        write_debug(msg);
        break;
      }
    case MPI_TAG_COMMUNICATION: 
      {
        int command;
        MPI_Recv(&command, 1, MPI_INT, status.MPI_SOURCE, 
             MPI_TAG_COMMUNICATION, MPI_COMM_WORLD, &status2);
        assert(status.MPI_SOURCE == status2.MPI_SOURCE);
        assert(status.MPI_TAG == status2.MPI_TAG);
        
        switch (command) {
        case MPI_CORRELATION_READY: {
          write_debug("Correlation finished, stopping.");
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
