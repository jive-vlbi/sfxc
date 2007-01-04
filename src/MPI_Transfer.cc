#include "MPI_Transfer.h"

MPI_Transfer::MPI_Transfer() {
}

  
void 
MPI_Transfer::send_general_parameters(int rank) {
  int size = 1024, position;
  char buff[size];
  
  // RunPrms
  MPI_Pack(&RunPrms.messagelvl, 1, MPI_INT, buff, size, &position, MPI_COMM_WORLD) 
  MPI_Pack(&RunPrms.interactive, 1, MPI_INT, buff, size, &position, MPI_COMM_WORLD) 
  MPI_Pack(&RunPrms.runoption, 1, MPI_INT, buff, size, &position, MPI_COMM_WORLD) 
}

void 
MPI_Transfer::receive_general_parameters(MPI_Status &status) {
  MPI_Status status2;
  
  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  assert(size > 0);
  char buff[size];
  MPI_Recv(&buff, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);

  std::cout << "MPI_MSG_TEXT_MESSAGE: " << message << std::endl;
   
}
