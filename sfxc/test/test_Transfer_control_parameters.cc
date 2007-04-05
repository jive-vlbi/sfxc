/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <types.h>
#include <iostream>
#include <assert.h>

#include "delayTable.h"
#include <MPI_Transfer.h>

// Defines bufTime
#include "constPrms.h"

#include <utils.h>
#include "Log_writer_cout.h"

#include <constPrms.h>
#include <runPrms.h>
#include <genPrms.h>
#include <staPrms.h>
RunP  RunPrms;
GenP  GenPrms;
StaP  StaPrms[NstationsMax];
INT64 sliceStartByte[NstationsMax][NprocessesMax];
INT64 sliceStartTime [NprocessesMax];
INT64 sliceStopTime  [NprocessesMax];
INT64 sliceTime;
int seed;

// MPI
int numtasks, rank;

void check_control_parameters() {
  if (rank==0) {
    //std::cout << "check_control_parameters()" << std::endl;
    MPI_Transfer mpi_transfer;
    mpi_transfer.send_general_parameters(1);
    
  } else {
    RunP  RunPrms2 = RunPrms;
    GenP  GenPrms2 = GenPrms;
    StaP  StaPrms2[NstationsMax];
    
    MPI_Status status;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    assert(status.MPI_TAG == MPI_TAG_CONTROL_PARAM);

    MPI_Transfer mpi_transfer;
    mpi_transfer.receive_general_parameters(status, RunPrms2, GenPrms2, StaPrms2);

    // RunPrms
    if (RunPrms.get_messagelvl() != RunPrms2.get_messagelvl()) {
      std::cout << "MPI RunPrms: messagelevel differs" << std::endl;
    }
    if (RunPrms.get_interactive() != RunPrms2.get_interactive()) {
      std::cout << "MPI RunPrms: interactive differs" << std::endl;
    }
    if (RunPrms.get_runoption() != RunPrms2.get_runoption()) {
      std::cout << "MPI RunPrms: interactive differs" << std::endl;
    }

    // GenPrms
    if (strcmp(GenPrms.get_experiment(), GenPrms2.get_experiment())) {
      std::cout << "MPI GenPrms: experiment differs" << std::endl;
    }
    if (GenPrms.get_yst() != GenPrms2.get_yst()) {
      std::cout << "MPI GenPrms: yst differs" << std::endl;
    }
    if (GenPrms.get_dst() != GenPrms2.get_dst()) {
      std::cout << "MPI GenPrms: dst differs" << std::endl;
    }
    if (GenPrms.get_hst() != GenPrms2.get_hst()) {
      std::cout << "MPI GenPrms: hst differs" << std::endl;
    }
    if (GenPrms.get_mst() != GenPrms2.get_mst()) {
      std::cout << "MPI GenPrms: mst differs" << std::endl;
    }
    if (GenPrms.get_sst() != GenPrms2.get_sst()) {
      std::cout << "MPI GenPrms: sst differs" << std::endl;
    }
//    if (GenPrms.get_ysp() != GenPrms2.get_ysp()) {
//      std::cout << "MPI GenPrms: ysp differs" << std::endl;
//    }
//    if (GenPrms.get_dsp() != GenPrms2.get_dsp()) {
//      std::cout << "MPI GenPrms: dsp differs" << std::endl;
//    }
//    if (GenPrms.get_hsp() != GenPrms2.get_hsp()) {
//      std::cout << "MPI GenPrms: hsp differs" << std::endl;
//    }
//    if (GenPrms.get_msp() != GenPrms2.get_msp()) {
//      std::cout << "MPI GenPrms: msp differs" << std::endl;
//    }
//    if (GenPrms.get_ssp() != GenPrms2.get_ssp()) {
//      std::cout << "MPI GenPrms: ssp differs" << std::endl;
//    }
    if (strcmp(GenPrms.get_outdir(), GenPrms2.get_outdir())) {
      std::cout << "MPI GenPrms: outdir differs" << std::endl;
    }
    if (strcmp(GenPrms.get_logfile(), GenPrms2.get_logfile())) {
      std::cout << "MPI GenPrms: logfile differs" << std::endl;
    }
    if (strcmp(GenPrms.get_corfile(), GenPrms2.get_corfile())) {
      std::cout << "MPI GenPrms: corfile differs" << std::endl;
    }
    
    if (GenPrms.get_bwin() != GenPrms2.get_bwin()) {
      std::cout << "MPI GenPrms: bwin differs" << std::endl;
    }
    if (GenPrms.get_lsegm() != GenPrms2.get_lsegm()) {
      std::cout << "MPI GenPrms: lsegm differs" << std::endl;
    }
    if (GenPrms.get_foffset() != GenPrms2.get_foffset()) {
      std::cout << "MPI GenPrms: foffset differs" << std::endl;
    }
    if (GenPrms.get_cde() != GenPrms2.get_cde()) {
      std::cout << "MPI GenPrms: cde differs" << std::endl;
    }
    if (GenPrms.get_mde() != GenPrms2.get_mde()) {
      std::cout << "MPI GenPrms: mde differs" << std::endl;
    }
    if (GenPrms.get_rde() != GenPrms2.get_rde()) {
      std::cout << "MPI GenPrms: rde differs" << std::endl;
    }

    if (GenPrms.get_filter() != GenPrms2.get_filter()) {
      std::cout << "MPI GenPrms: filter differs" << std::endl;
    }
    if (GenPrms.get_bwfl() != GenPrms2.get_bwfl()) {
      std::cout << "MPI GenPrms: bwfl differs" << std::endl;
    }
    if (GenPrms.get_startf() != GenPrms2.get_startf()) {
      std::cout << "MPI GenPrms: startf differs" << std::endl;
    }
    if (GenPrms.get_deltaf() != GenPrms2.get_deltaf()) {
      std::cout << "MPI GenPrms: deltaf differs" << std::endl;
    }
    if (GenPrms.get_ovrfl() != GenPrms2.get_ovrfl()) {
      std::cout << "MPI GenPrms: overfl differs" << std::endl;
    }

    if (GenPrms.get_n2fft() != GenPrms2.get_n2fft()) {
      std::cout << "MPI GenPrms: n2fft differs" << std::endl;
    }
    if (GenPrms.get_ovrlp() != GenPrms2.get_ovrlp()) {
      std::cout << "MPI GenPrms: overlp differs" << std::endl;
    }
//    if (GenPrms.get_nsamp2avg() != GenPrms2.get_nsamp2avg()) {
//      std::cout << "MPI GenPrms: nsamp2avg differs" << std::endl;
//    }
    if (GenPrms.get_pad() != GenPrms2.get_pad()) {
      std::cout << "MPI GenPrms: pad differs" << std::endl;
    }

    if (GenPrms.get_usStart() != GenPrms2.get_usStart()) {
      std::cout << "MPI GenPrms: usStart differs" << std::endl;
    }
//    if (GenPrms.get_usStop() != GenPrms2.get_usStop()) {
//      std::cout << "MPI GenPrms: usStop differs" << std::endl;
//    }
//    if (GenPrms.get_usEarliest() != GenPrms2.get_usEarliest()) {
//      std::cout << "MPI GenPrms: usEarliest differs" << std::endl;
//    }
//    if (GenPrms.get_usLatest() != GenPrms2.get_usLatest()) {
//      std::cout << "MPI GenPrms: usLatest differs" << std::endl;
//    }
    
    for (int i=0; i<GenPrms.get_nstations(); i++) {
      if (strcmp(StaPrms[i].get_stname(), StaPrms2[i].get_stname())) {
        std::cout << "MPI StaPrms[" << i << "]: stname differs" << std::endl;
        std::cout << "stname: \"" 
                  << StaPrms[i].get_stname() << "\" vs. " << std::endl
                  << "        \""  
                  << StaPrms2[i].get_stname() << "\"" << std::endl;
      }

      if (StaPrms[i].get_datatype() != StaPrms2[i].get_datatype()) {
        std::cout << "MPI StaPrms[" << i << "]: datatype differs" << std::endl;
      }
      if (StaPrms[i].get_tbr() != StaPrms2[i].get_tbr()) {
        std::cout << "MPI StaPrms[" << i << "]: tbr differs" << std::endl;
      }
      if (StaPrms[i].get_fo() != StaPrms2[i].get_fo()) {
        std::cout << "MPI StaPrms[" << i << "]: fo differs" << std::endl;
      }
      if (StaPrms[i].get_bps() != StaPrms2[i].get_bps()) {
        std::cout << "MPI StaPrms[" << i << "]: bps differs" << std::endl;
      }
      if (StaPrms[i].get_tphs() != StaPrms2[i].get_tphs()) {
        std::cout << "MPI StaPrms[" << i << "]: tphs differs" << std::endl;
      }
      if (StaPrms[i].get_nhs() != StaPrms2[i].get_nhs()) {
        std::cout << "MPI StaPrms[" << i << "]: nhs differs" << std::endl;
      }
      if (StaPrms[i].get_boff() != StaPrms2[i].get_boff()) {
        std::cout << "MPI StaPrms[" << i << "]: boff differs" << std::endl;
      }
      if (StaPrms[i].get_synhs1() != StaPrms2[i].get_synhs1()) {
        std::cout << "MPI StaPrms[" << i << "]: synhs1 differs" << std::endl;
      }
      if (StaPrms[i].get_synhs2() != StaPrms2[i].get_synhs2()) {
        std::cout << "MPI StaPrms[" << i << "]: synhs2 differs" << std::endl;
      }
      if (StaPrms[i].get_mod() != StaPrms2[i].get_mod()) {
        std::cout << "MPI StaPrms[" << i << "]: mod differs" << std::endl;
      }
      if (StaPrms[i].get_rndhdr() != StaPrms2[i].get_rndhdr()) {
        std::cout << "MPI StaPrms[" << i << "]: rndhdr differs" << std::endl;
      }

      if (strcmp(StaPrms[i].get_mk4file(), StaPrms2[i].get_mk4file())) {
        std::cout << "MPI StaPrms[" << i << "]: mk4file differs" << std::endl;
        std::cout << "mk4file: \"" 
                  << StaPrms[i].get_mk4file() << "\" vs. " << std::endl
                  << "         \""  
                  << StaPrms2[i].get_mk4file() << "\"" << std::endl;
      }
      if (strcmp(StaPrms[i].get_hdrmap(), StaPrms2[i].get_hdrmap())) {
        std::cout << "MPI StaPrms[" << i << "]: hdrmap differs" << std::endl;
        std::cout << "hdrmap: \"" 
                  << StaPrms[i].get_hdrmap() << "\" vs. " << std::endl
                  << "        \""  
                  << StaPrms2[i].get_hdrmap() << "\"" << std::endl;
      }
      if (strcmp(StaPrms[i].get_modpat(), StaPrms2[i].get_modpat())) {
        std::cout << "MPI StaPrms[" << i << "]: modpat differs" << std::endl;
        std::cout << "modpat: \"" 
                  << StaPrms[i].get_modpat() << "\" vs. " << std::endl
                  << "        \""  
                  << StaPrms2[i].get_modpat() << "\"" << std::endl;
      }
      if (strcmp(StaPrms[i].get_delaytable(), StaPrms2[i].get_delaytable())) {
        std::cout << "MPI StaPrms[" << i << "]: delaytable differs" << std::endl;
        std::cout << "delaytable: \"" 
                  << StaPrms[i].get_delaytable() << "\" vs. " << std::endl
                  << "            \"" 
                  << StaPrms2[i].get_delaytable() << "\"" << std::endl;
      }
      if (strcmp(StaPrms[i].get_phasetable(), StaPrms2[i].get_phasetable())) {
        std::cout << "MPI StaPrms[" << i << "]: phasetable differs" << std::endl;
        std::cout << "phasetable: \"" 
                  << StaPrms[i].get_phasetable() << "\" vs. " << std::endl
                  << "            \"" 
                  << StaPrms2[i].get_phasetable() << "\"" << std::endl;
      }

      if (StaPrms[i].get_loobs() != StaPrms2[i].get_loobs()) {
        std::cout << "MPI StaPrms[" << i << "]: loobs differs" << std::endl;
      }
      for (int j=0; j<fomax; j++) {
        if (StaPrms[i].get_signBS()[j] != StaPrms2[i].get_signBS()[j]) {
          std::cout << "MPI StaPrms[" << i << "]: signBS()[j] differs" << std::endl;
        }
        if (StaPrms[i].get_magnBS()[j] != StaPrms2[i].get_magnBS()[j]) {
          std::cout << "MPI StaPrms[" << i << "]: magnBS()[j] differs" << std::endl;
        }
      }
    }
  }
}

void check_delay_table(char *filename_delay_table) {
  if (rank==0) {
    std::cout << "check delay: " << filename_delay_table << std::endl;
  }

  // Read delay_table:
  DelayTable delayTable;
  delayTable.set_cmr(GenPrms);
  delayTable.readDelayTable(filename_delay_table);
  int sn = 134; // Some random value

  MPI_Transfer transfer;
  if (rank==0) {
    { // assignment
      DelayTable delayTable2 = delayTable;
      assert(delayTable == delayTable2);
    }
  
    { // copy constructor
      DelayTable delayTable2(delayTable);
      assert(delayTable == delayTable2);
    }
    
    transfer.send_delay_table(delayTable,sn,1);
  } else {
    DelayTable delayTable2;
    delayTable2.set_cmr(GenPrms);
    MPI_Status status;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    int sn2;
    transfer.receive_delay_table(status,delayTable2,sn2);
    assert(delayTable == delayTable2);
    assert(sn == sn2);
  }
}


int main(int argc, char *argv[]) {
  Log_writer_cout log_writer(-1);
  //initialisation
  int status = MPI_Init(&argc,&argv);
  if (status != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, status);
  }

  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  // Initialise correlator node
  assert(argc==2);
  char *control_file = argv[1];
  if (initialise_control(control_file, log_writer, RunPrms, GenPrms, StaPrms) != 0) {
    std::cout << "Initialisation using control file failed" << std::endl;
    return 1;
  }
  
  check_control_parameters();

  for (int i=0; i<GenPrms.get_nstations(); i++) {
    MPI_Barrier( MPI_COMM_WORLD );
    check_delay_table(StaPrms[i].get_delaytable());
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
