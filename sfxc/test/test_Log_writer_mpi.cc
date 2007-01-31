#include <iostream>
#include <assert.h>

#include "Node.h"
#include "Log_writer_cout.h"
#include "Log_writer_mpi.h"
#include "Log_controller.h"

class Test_log_node : public Node {
public:
  Test_log_node(int rank) : Node(rank), log_controller(log_writer) {
    add_controller(&log_controller);
  }
private:
  Log_writer_cout log_writer;
  Log_controller log_controller;
};

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
    Test_log_node node(rank);
    node.start();
  } else {
    Log_writer_mpi writer;
    writer.set_rank(rank);
    writer << "a\nb\nc\n";
    int i=0;
    MPI_Send(&i, 1, MPI_INT32, 0, MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
  }


  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
