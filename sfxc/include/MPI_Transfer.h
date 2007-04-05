/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

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
  
  void send_delay_table(DelayTable &table, int sn, int rank);
  void receive_delay_table(MPI_Status &status, DelayTable &table, int &sn);
};

#endif /*MPI_TRANSFER_H_*/
