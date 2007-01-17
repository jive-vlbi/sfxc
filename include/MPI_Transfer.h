#ifndef MPI_TRANSFER_H_
#define MPI_TRANSFER_H_

#include "types.h"
#include "sfxc_mpi.h"
#include "delayTable.h"
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
/** Transfer special classes for the sfxc using MPI
 **/
class MPI_Transfer
{
public:
	MPI_Transfer();
	
  void send_general_parameters(int rank);
  void receive_general_parameters(MPI_Status &status, RunP &RunPrms, GenP &GenPrms, StaP StaPrms[]);
  
  void send_delay_table(DelayTable &table, int rank);
  void receive_delay_table(MPI_Status &status, DelayTable &table);
};

#endif /*MPI_TRANSFER_H_*/
