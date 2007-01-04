#ifndef MPI_TRANSFER_H_
#define MPI_TRANSFER_H_

#include "types.h"
/** Transfer special classes for the sfxc using MPI
 **/
class MPI_Transfer
{
public:
	MPI_Transfer();
	
  void send_general_parameters(int rank);
  void receive_general_parameters(MPI_Status &status);
};

#endif /*MPI_TRANSFER_H_*/
