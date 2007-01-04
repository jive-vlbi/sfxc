#include "MPI_Transfer.h"
#include <types.h>
#include <assert.h>
#include <iostream>

/// TODO: NGHK: REMOVE
#include <constPrms.h>
#include <runPrms.h>
#include <genPrms.h>
#include <staPrms.h>
extern RunP  RunPrms;
extern GenP  GenPrms;
extern StaP  StaPrms[NstationsMax];
extern INT64 sliceStartByte[NstationsMax][NprocessesMax];
extern INT64 sliceStartTime [NprocessesMax];
extern INT64 sliceStopTime  [NprocessesMax];
extern INT64 sliceTime;

MPI_Transfer::MPI_Transfer() {
}

  
void 
MPI_Transfer::send_general_parameters(int rank) {
  int size = 1024, position=0;
  char buffer[size];
  
  // RunPrms
  MPI_Pack(&RunPrms.messagelvl, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD); 
  MPI_Pack(&RunPrms.interactive, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&RunPrms.runoption, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  
  // GenPrms
  MPI_Pack(&GenPrms.nstations, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.bwin, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.lsegm, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.foffset, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.cde, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.mde, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.rde, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  
  MPI_Pack(&GenPrms.filter, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.bwfl, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.startf, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.deltaf, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.ovrfl, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);

  MPI_Pack(&GenPrms.n2fft, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.ovrlp, 1, MPI_FLOAT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.nsamp2avg, 1, MPI_LONG, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.pad, 1, MPI_INT, buffer, size, &position, MPI_COMM_WORLD);
  
  // add data for the stations:
  for (int station=0; station<GenPrms.get_nstations(); station++) {
    MPI_Pack(&(StaPrms[station].datatype), 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&(StaPrms[station].tbr), 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].fo, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].bps, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].nhs, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].tphs, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].boff, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].synhs1, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].synhs2, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].mod, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].rndhdr, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].signBS, fomax, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].magnBS, fomax, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].hs, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].hm, 1, MPI_INT, 
             buffer, size, &position, MPI_COMM_WORLD);

    MPI_Pack(&StaPrms[station].loobs, 1, MPI_LONG, 
             buffer, size, &position, MPI_COMM_WORLD);
  }

  assert(position < size);
  
  MPI_Send(buffer, position, MPI_PACKED, rank, MPI_TAG_CONTROL_PARAM, MPI_COMM_WORLD);
}

void 
MPI_Transfer::receive_general_parameters(MPI_Status &status) {
  MPI_Status status2;
  
  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  std::cout << "receive_general_parameters: Received size is : " << size << std::endl;
  assert(size > 0);
  char buffer[size];
  MPI_Recv(&buffer, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);

  int position = 0; 
  
  MPI_Unpack(buffer, 1000, &position, &RunPrms.messagelvl, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, 1000, &position, &RunPrms.interactive, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, 1000, &position, &RunPrms.runoption, 1, MPI_INT, MPI_COMM_WORLD);

//  int size = 1024;
//  MPI_Status status2;
//  char buffer[size];
//  int position = 0;
//  MPI_Recv(buffer, size, MPI_PACKED, 0, MPI_TAG_CONTROL_PARAM, MPI_COMM_WORLD, &status2);
//  
  MPI_Unpack(buffer, size, &position, &GenPrms.nstations, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.bwin, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.lsegm, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.foffset, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.cde, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.mde, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.rde, 1, MPI_INT, MPI_COMM_WORLD);
  
  MPI_Unpack(buffer, size, &position, &GenPrms.filter, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.bwfl, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.startf, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.deltaf, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.ovrfl, 1, MPI_INT, MPI_COMM_WORLD);

  MPI_Unpack(buffer, size, &position, &GenPrms.n2fft, 1, MPI_INT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.ovrlp, 1, MPI_FLOAT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.nsamp2avg, 1, MPI_LONG, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.pad, 1, MPI_INT, MPI_COMM_WORLD);

  for (int station=0; station<GenPrms.get_nstations(); station++) {
    MPI_Unpack(buffer, size, &position, &(StaPrms[station].datatype), 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &(StaPrms[station].tbr), 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].fo, 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].bps, 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].nhs, 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].tphs, 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].boff, 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].synhs1, 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].synhs2, 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].mod, 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].rndhdr, 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].signBS, fomax, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].magnBS, fomax, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].hs, 1, MPI_INT, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].hm, 1, MPI_INT, 
             MPI_COMM_WORLD);

    MPI_Unpack(buffer, size, &position, &StaPrms[station].loobs, 1, MPI_LONG, 
             MPI_COMM_WORLD);
  }
  
  //RunPrms.messagelvl = 2;

    //check control parameters, optionally show them
  if (RunPrms.check_params() != 0) {
    std::cerr << "ERROR: Run control parameter, program aborted.\n";
    return;
  }

  //check general control parameters, optionally show them
  if (GenPrms.check_params() != 0) {
    std::cerr << "ERROR: General control parameter, program aborted.\n";
    return;
  }
  

  //get the number of stations
  int Nstations = GenPrms.get_nstations();

  //check station control parameters, optionally show them
  for (int i=0; i<Nstations; i++){
    if (StaPrms[i].check_params() != 0 ) {
      std::cerr << "ERROR: Station control parameter, program aborted.\n";
      return;
    }
  }

   
}
