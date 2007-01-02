/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Source       sfxc.cc
Title        software FX correlator
Author       RHJ Oerlemans
Started      20060901
Last change  20061114

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
#undef SEEK_SET
#undef SEEK_END
#undef SEEK_CUR
#include <mpi.h>

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
//declarations for offsets
INT64 sliceStartByte[NstationsMax][NprocessesMax];
INT64 sliceStartTime [NprocessesMax];
INT64 sliceStopTime  [NprocessesMax];
INT64 sliceTime;


//return value 0 when no errors occurred
int main(int argc, char *argv[])
{
/*
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " <ctrl-file>" << std::endl;
    exit(1);
  }
*/

  //declarations
  char   ctrlFile[lineLength]; // control file name
  int    i, Nstations;
  
  int status, numtasks, rank;

  // seed the random number generator (global variable!)
  seed = (UINT32) time((time_t *)NULL);
  //TODO 13-12-2006: remove next line after tests
  seed = SEED;
  cout << "WARNING seed=" << seed << endl;

  //do the mpi initialisation
  status = MPI_Init(&argc,&argv);
  if (status != MPI_SUCCESS) {
    cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, status);
  }
  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  if (rank==0) {
    std::cout << "sfcx v" << PACKAGE_VERSION << std::endl;
  }

  if(numtasks < 1) //entered at the command line
  {
    cerr << "number of tasks is smaller than 1, program aborted\n";
    return -1;
  }
  
  //set the control file name
  strcpy(ctrlFile,argv[1]);

  //parse control file for run parameters
  if (RunPrms.parse_ctrlFile(ctrlFile) != 0) {
    cerr << "ERROR: Control file "<< ctrlFile <<", program aborted.\n";
    return -1;
  }

  //show version information and control file info
  if (RunPrms.get_messagelvl() > 0)
    cout << "\nSource " << __FILE__ << " compiled at: "
         << __DATE__ << " " <<__TIME__ << "\n\n"
         << "Control file name "  <<  ctrlFile << "\n\n";
  
  //check control parameters, optionally show them
  if (RunPrms.check_params() != 0) {
    cerr << "ERROR: Run control parameter, program aborted.\n";
    return -1;
  }
  
  if (RunPrms.get_interactive() && RunPrms.get_messagelvl()> 0 && numtasks == 1)
    askContinue();

  //parse control file for general parameters
  if (GenPrms.parse_ctrlFile(ctrlFile) != 0) {
    cerr << "ERROR: Control file "<< ctrlFile <<", program aborted.\n";
    return -1;
  }

  //check general control parameters, optionally show them
  if (GenPrms.check_params() != 0) {
    cerr << "ERROR: General control parameter, program aborted.\n";
    return -1;
  }
  
  if (RunPrms.get_interactive() && RunPrms.get_messagelvl()> 0 && numtasks == 1)
    askContinue();

  //get the number of stations
  Nstations = GenPrms.get_nstations();
  
  //parse the control file for all station parameters
  for (i=0; i<Nstations; i++)
    if (StaPrms[i].parse_ctrlFile(ctrlFile,i) != 0 ) {
      cerr << "ERROR: Control file "<< ctrlFile <<", program aborted.\n";
      return -1;
    }
    
  //check station control parameters, optionally show them
  for (i=0; i<Nstations; i++){
    if (StaPrms[i].check_params() != 0 ) {
      cerr << "ERROR: Station control parameter, program aborted.\n";
      return -1;
    }
    if (RunPrms.get_interactive() && RunPrms.get_messagelvl()> 0 && numtasks == 1)
      askContinue();
  }  

  // NGHK: Has to be a pointer or a reference, 
  //       since Data_reader is an abstract class
  std::vector<Data_reader *> input_readers;
  for (int i=0; i<Nstations; i++) {
    input_readers.push_back(new Data_reader_file(StaPrms[i].get_mk4file()));
  }

  //Find Offsets
  if (FindOffsets(input_readers, numtasks, rank) !=0) {
    cerr << "ERROR: FindOffsets, program aborted.\n";
    return -1;
  }

  if ( RunPrms.get_runoption() == 1 ) {
    //Process data for rank (=process identifier)
    if (CorrelateBufs(rank, input_readers) != 0) {
      cerr << "ERROR: CorrelateBufs, program aborted.\n";
      return -1;
    }
  }

  //close the mpi stuff
  MPI_Finalize();

  return 0;

}

