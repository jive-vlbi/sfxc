/* Copyright (c) 2006 JIVE (Netherlands)
 * All rights reserved.
 * 
 * This file is part of sfxc (software FX correlator);
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Author(s): Ruud Oerlemans, 2006 
 *            Nico Kruithof, 2007
 * 
 * $Id$
 */

/*
TODO: add doxygen style comments in *.cc and *.h
TODO: review message levels in *.cc and *.h
TODO: compiler at huygens generates more warning, check and try to fix this

Description
sfxc correlates the single channel data from N stations recorded on linux
type harddisks. It can run on a multiprocessor machine or cluster.
When sfxc runs on M processors the data from each station is divided into
M chunks of equal length. All chunks from N stations but in the same
time interval are processed on a separate processor. After the correlation
of the time chunks the separate output files will be concatenated.

The diagram below shows how the data from 4 stations is processed
simultaneously on 3 processors. After the correlation the 3 correlator
product files are concatenated into one data file.

 ST0001 | ST0002 | ST0003 | ST0004 |
--------+--------+--------+--------+--------+                 +------------+
chunk 1 | data   | data   | data   | data   | ---> CORE_1 --->|CorProduct 1|
--------+--------+--------+--------+--------+                 +------------+
chunk 2 | data   | data   | data   | data   | ---> CORE_2 --->|CorProduct 2|
--------+--------+--------+--------+--------+                 +------------+
chunk 3 | data   | data   | data   | data   | ---> CORE_3 --->|CorProduct 3|
--------+--------+--------+--------+--------+                 +------------+


+------------+
|CorProduct 1| \
+------------+  \             +----------------+
|CorProduct 2| --> CORE  ---> |CorProduct 1,2,3|
+------------+  /             +----------------+
|CorProduct 3| /
+------------+

Input files:

control file.
-Contains all relevant information to run sfxc
-ascii file with keyword-value pairs. File is parsed

data files,
-one for each station
-supported data formats: Mk4 formatted data on linux type disks

delay table files
-one for each station


phase table files
-one for each station
-this table is only used in special cases like spacecraft tracking
-TODO the use of this table is not implemented yet

Output:
-log file.  TODO: writing of log file not implemented
-correlator file(s), one per time slice. TODO: concatenate into one output file

Usage:
mpirun -np numtasks sfxc control_file

mpirun        : command to run a MPI enabled application
-np           : command option indicating the number of processors
numtasks      : number of processors
sfxc          : the software correlator application
control_file  : file with parameter settings for sfxc

Flow of the program:
MPI settings
Read control file
Find Offsets
Process data
*/

#include <types.h>

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

//includes for system calls
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

//includes for mpi
//undef have to be before include <mpi.h>
//#undef SEEK_SET
//#undef SEEK_END
//#undef SEEK_CUR
//#include <mpi.h>

//constants
#include "constPrms.h"

//class and function definitions
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "InData.h"
#include "ProcessData.h"

#include <Data_reader_file.h>
#include <Data_writer_file.h>
#include <Log_writer_cout.h>

#define SEED 10

//global variables
//declaration and default settings run parameters
RunP RunPrms;
//declaration and default settings general parameters
GenP GenPrms;
//station parameters class, declaration and default settings
StaP StaPrms[NstationsMax];
// used for randomising numbers for Headers in Mk4 file
UINT32 seed;


//return value 0 when no errors occurred
int main(int argc, char *argv[])
{
  Log_writer_cout log_writer(0,false);
  set_log_writer(log_writer); 

  if (argc != 2) {
    log_writer << "usage: " << argv[0] << " <ctrl-file>" << std::endl;
    exit(1);
  }


  //declarations
  char   ctrlFile[lineLength]; // control file name
  int    i, Nstations;
  
  // seed the random number generator (global variable!)
  seed = (UINT32) time((time_t *)NULL);
  //TODO 13-12-2006: Fixed seeding for test purposes.
  //make fixed seeding or time based seeding a control file option
  seed = SEED;
  log_writer << std::endl << "WARNING seed=" << seed << endl;

  log_writer << "sfcx v" << PACKAGE_VERSION << std::endl;

  //set the control file name
  strcpy(ctrlFile,argv[1]);

  //parse control file for run parameters
  if (RunPrms.parse_ctrlFile(ctrlFile, log_writer) != 0) {
    log_writer << "ERROR: Control file "<< ctrlFile <<", program aborted.\n";
    return -1;
  }

  log_writer.set_messagelevel(RunPrms.get_messagelvl());
  log_writer.set_interactive(RunPrms.get_interactive());

  //show version information and control file info
  log_writer << "Source " << __FILE__ 
               << " compiled at: " << __DATE__ << " "<<__TIME__<<"\n\n";
  
  //check control parameters, optionally show them
  if (RunPrms.check_params(log_writer) != 0) {
    log_writer.message(0,"ERROR: Run control parameter, program aborted.\n");
    return -1;
  }
  
  log_writer.ask_continue();

  //parse control file for general parameters
  if (GenPrms.parse_ctrlFile(ctrlFile, log_writer) != 0) {
    log_writer << "ERROR: Control file " << ctrlFile <<", program aborted.\n";
    return -1;
  }

  //check general control parameters, optionally show them
  if (GenPrms.check_params(log_writer) != 0) {
    log_writer.message(0,"ERROR: General control parameter, program aborted.\n");
    return -1;
  }
  
  log_writer.ask_continue();

  // Set the output writer
  Data_writer_file data_writer(GenPrms.get_corfile());
  set_data_writer(data_writer); 


  //get the number of stations
  Nstations = GenPrms.get_nstations();
  
  //parse the control file for all station parameters
  for (i=0; i<Nstations; i++)
    if (StaPrms[i].parse_ctrlFile(ctrlFile, i, log_writer) != 0 ) {
      log_writer << "ERROR: Control file " << ctrlFile <<", program aborted.\n";
      return -1;
    }
    
  //check station control parameters, optionally show them
  for (i=0; i<Nstations; i++){
    if (StaPrms[i].check_params(log_writer) != 0 ) {
      log_writer.message(0,"ERROR: Station control parameter, program aborted.\n");
      return -1;
    }
    log_writer.ask_continue();
  }  
  

  // NGHK: Has to be a pointer or a reference, 
  //       since Data_reader is an abstract class
  std::vector<Data_reader *> input_readers;
  for (int i=0; i<Nstations; i++) {
    input_readers.push_back(new Data_reader_file(StaPrms[i].get_mk4file()));
  }

//  //Find Offsets
//  if (FindOffsets(input_readers, 1, 0) !=0) {
//    log_writer.message(0,"ERROR: FindOffsets, program aborted.\n");
//    return -1;
//  }

  if ( RunPrms.get_runoption() == 1 ) {
    //Process data for rank (=process identifier)
    if (CorrelateBufs(input_readers) != 0) {
      log_writer.message(0,"ERROR: CorrelateBufs, program aborted.\n");
      return -1;
    }
  }

  return 0;

}

