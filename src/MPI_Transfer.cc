/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "MPI_Transfer.h"
#include <types.h>
#include <assert.h>
#include <iostream>

/// TODO: NGHK: REMOVE
#include <constPrms.h>
#include <runPrms.h>
#include <genPrms.h>
#include <staPrms.h>
//extern RunP  RunPrms;
//extern GenP  GenPrms;
//extern StaP  StaPrms[NstationsMax];
extern int64_t sliceStartByte[NstationsMax][NprocessesMax];
extern int64_t sliceStartTime [NprocessesMax];
extern int64_t sliceStopTime  [NprocessesMax];
extern int64_t sliceTime;

MPI_Transfer::MPI_Transfer() {
}

  
void 
MPI_Transfer::send_general_parameters(int rank,
                                      RunP &RunPrms, GenP &GenPrms, StaP StaPrms[]) {
  int size = 1024, position=0;
  char buffer[size];
  
  // RunPrms
  MPI_Pack(&RunPrms.messagelvl, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD); 
  MPI_Pack(&RunPrms.interactive, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&RunPrms.runoption, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&RunPrms.ref_station1, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&RunPrms.ref_station2, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  
  // GenPrms
  MPI_Pack(&GenPrms.yst, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.dst, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.hst, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.mst, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.sst, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);

  MPI_Pack(&GenPrms.duration, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.skyfreq, 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD);

  MPI_Pack(&GenPrms.nstations, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.bwin, 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.lsegm, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.foffset, 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.cde, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.mde, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.rde, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  
  MPI_Pack(&GenPrms.filter, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.bwfl, 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.startf, 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.deltaf, 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.ovrfl, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);

  MPI_Pack(&GenPrms.n2fft, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.ovrlp, 1, MPI_FLOAT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.time2avg, 1, MPI_FLOAT, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.pad, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  MPI_Pack(&GenPrms.rndhdr, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);
  
  MPI_Pack(&GenPrms.usStart, 1, MPI_INT64, buffer, size, &position, MPI_COMM_WORLD);

  MPI_Pack(&GenPrms.sideband, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD);

  // add data for the stations:
  for (int station=0; station<GenPrms.get_nstations(); station++) {
    MPI_Pack(&(StaPrms[station].datatype), 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&(StaPrms[station].tbr), 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].fo, 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].bps, 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].nhs, 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].tphs, 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].boff, 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].synhs1, 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].synhs2, 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].mod, 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].signBS, fomax, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].magnBS, fomax, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].hs, 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].hm, 1, MPI_INT32, 
             buffer, size, &position, MPI_COMM_WORLD);
    MPI_Pack(&StaPrms[station].loobs, 1, MPI_INT64, 
             buffer, size, &position, MPI_COMM_WORLD);
  }

  assert(position <= size);
  
  MPI_Send(buffer, position, MPI_PACKED, rank, MPI_TAG_CONTROL_PARAM, MPI_COMM_WORLD);
}

void 
MPI_Transfer::receive_general_parameters(MPI_Status &status, 
                                         RunP &RunPrms,
                                         GenP &GenPrms,
                                         StaP StaPrms[]) {
  MPI_Status status2;
  
  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  assert(size > 0);
  char buffer[size];
  MPI_Recv(&buffer, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);

  int position = 0; 
  
  MPI_Unpack(buffer, size, &position, &RunPrms.messagelvl, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &RunPrms.interactive, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &RunPrms.runoption, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &RunPrms.ref_station1, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &RunPrms.ref_station2, 1, MPI_INT32, MPI_COMM_WORLD);

  MPI_Unpack(buffer, size, &position, &GenPrms.yst, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.dst, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.hst, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.mst, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.sst, 1, MPI_INT32, MPI_COMM_WORLD);

  MPI_Unpack(buffer, size, &position, &GenPrms.duration, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.skyfreq, 1, MPI_DOUBLE, MPI_COMM_WORLD);
  
  MPI_Unpack(buffer, size, &position, &GenPrms.nstations, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.bwin, 1, MPI_DOUBLE, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.lsegm, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.foffset, 1, MPI_DOUBLE, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.cde, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.mde, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.rde, 1, MPI_INT32, MPI_COMM_WORLD);
  
  MPI_Unpack(buffer, size, &position, &GenPrms.filter, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.bwfl, 1, MPI_DOUBLE, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.startf, 1, MPI_DOUBLE, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.deltaf, 1, MPI_DOUBLE, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.ovrfl, 1, MPI_INT32, MPI_COMM_WORLD);

  MPI_Unpack(buffer, size, &position, &GenPrms.n2fft, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.ovrlp, 1, MPI_FLOAT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.time2avg, 1, MPI_FLOAT, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &GenPrms.pad, 1, MPI_INT32, MPI_COMM_WORLD);

  MPI_Unpack(buffer, size, &position, &GenPrms.rndhdr, 1, MPI_INT32, 
             MPI_COMM_WORLD);


  MPI_Unpack(buffer, size, &position, &GenPrms.usStart, 1, MPI_INT64, MPI_COMM_WORLD);

  MPI_Unpack(buffer, size, &position, &GenPrms.sideband, 1, MPI_INT32, MPI_COMM_WORLD);
  
  for (int station=0; station<GenPrms.get_nstations(); station++) {
    MPI_Unpack(buffer, size, &position, &(StaPrms[station].datatype), 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &(StaPrms[station].tbr), 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].fo, 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].bps, 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].nhs, 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].tphs, 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].boff, 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].synhs1, 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].synhs2, 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].mod, 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].signBS, fomax, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].magnBS, fomax, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].hs, 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].hm, 1, MPI_INT32, 
             MPI_COMM_WORLD);
    MPI_Unpack(buffer, size, &position, &StaPrms[station].loobs, 1, MPI_INT64, 
             MPI_COMM_WORLD);
  }
  
  assert(position == size);
}

void 
MPI_Transfer::send_delay_table(DelayTable &table, int sn, int rank) {
  int size = 3*sizeof(int64_t) + 4*sizeof(int32_t) + table.ndel*12*sizeof(double); 
  int position=0;
  char buffer[size];

  // First, set the station number
  MPI_Pack(&sn, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD); 

  // Scalars
  MPI_Pack(&table.ndel, 1, MPI_INT64, buffer, size, &position, MPI_COMM_WORLD); 
  MPI_Pack(&table.startDT, 1, MPI_INT64, buffer, size, &position, MPI_COMM_WORLD); 
  MPI_Pack(&table.stepDT, 1, MPI_INT64, buffer, size, &position, MPI_COMM_WORLD); 

  MPI_Pack(&table.cde, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD); 
  MPI_Pack(&table.mde, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD); 
  MPI_Pack(&table.rde, 1, MPI_INT32, buffer, size, &position, MPI_COMM_WORLD); 

  // Arrays
  for (int i=0; i<table.ndel; i++) {
    MPI_Pack(&(table.cA[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.cB[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.cC[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.mA[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.mB[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.mC[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.rA[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.rB[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.rC[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.fA[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.fB[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
    MPI_Pack(&(table.fC[i]), 1, MPI_DOUBLE, buffer, size, &position, MPI_COMM_WORLD); 
  }
  assert(position <= size);

  MPI_Send(buffer, position, MPI_PACKED, rank, MPI_TAG_DELAY_TABLE, MPI_COMM_WORLD);
}

void 
MPI_Transfer::receive_delay_table(MPI_Status &status, DelayTable &table, int &sn) {
  MPI_Status status2;
  
  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  assert(size > 0);
  char buffer[size];
  MPI_Recv(&buffer, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);

  int position = 0; 

  // First, get the station number
  MPI_Unpack(buffer, size, &position, &sn, 1, MPI_INT32, MPI_COMM_WORLD);
  

  // Scalars
  MPI_Unpack(buffer, size, &position, &table.ndel, 1, MPI_INT64, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &table.startDT, 1, MPI_INT64, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &table.stepDT, 1, MPI_INT64, MPI_COMM_WORLD);

  MPI_Unpack(buffer, size, &position, &table.cde, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &table.mde, 1, MPI_INT32, MPI_COMM_WORLD);
  MPI_Unpack(buffer, size, &position, &table.rde, 1, MPI_INT32, MPI_COMM_WORLD);

  table.reserve_data();
  
  // Arrays
  for (int i=0; i<table.ndel; i++) {
    MPI_Unpack(buffer, size, &position, &table.cA[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.cB[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.cC[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.mA[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.mB[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.mC[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.rA[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.rB[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.rC[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.fA[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.fB[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
    MPI_Unpack(buffer, size, &position, &table.fC[i], 1, MPI_DOUBLE, MPI_COMM_WORLD); 
  }
  assert(position == size);
}
