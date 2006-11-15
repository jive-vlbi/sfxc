/*
Source       sfxc01.cc
Title        software FX correlator
Author       RHJ Oerlemans
Started      20060901
Last change  20060912


Description
sfxc01 correlates the single channel data from N stations recorded on linux
type harddisks. It can run on a multiprocessor machine or cluster.
When sfxc01 runs on M processors the data from each station is divided into
M chunks of equal length. All chunks from N stations but in the same
time interval are processed on a separate processor. After the correlation
of the time chunks the separate output files can be concatenated.

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
-Contains all relevant information to run sfxc01
-ascii file with keyword-value pairs. File is parsed

data files,
-one for each station
-supported data formats: Mk4 formatted data on linux type disks

delay table files
-one for each station
-

phase table files
-one for each station

Output:
log file
correlator file(s)

Usage:
sfxc01  control_file [ro ml nc i] or
sfxc01t control_file [ro ml nc i]
command line options between [] are optional:
  ro: run option ro=[r0|r1] default=1, complete
  ml: message level ml=[m0|m1|m2]. default=0 no messages except for errors
  nc: nr of cores   nc=[n1|n2|n3|...] default 1 core
  i : interactive when present. default not interactive mode
The command line options can also be set in the control file.
These values override the ones set on the command line.

Flow of the program:
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

//constants
#include "constPrms.h"

//class and function definitions
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "InData.h"
#include "ProcessData.h"

#include <vector>
#include <Input_reader_file.h>


//global variables
//declaration and default settings run parameters
RunP RunPrms;
//declaration and default settings general parameters
GenP GenPrms;
//station parameters class, declaration and default settings
StaP StaPrms[NstationsMax];
// used for randomising numbers for Headers in Mk4 file
UINT32 seed;
//PI
double PI=4.0*atan(1.0);
//declarations for offsets
INT64 sliceStartByte[NstationsMax][NcoresMax];
// INT64 sliceStopByte [NstationsMax][NcoresMax];
INT64 sliceStartTime [NcoresMax];
//INT64 sliceStopTime  [NcoresMax];
INT64 sliceTime;


int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " <ctrl-file>" << std::endl;
    exit(1);
  }

  //declarations
  char   ctrlFile[lineLength]; // control file name
  int    i, Nstations;
  
  int numtasks=1;

  // seed the random number generator (global variable!)
  seed = (UINT32) time((time_t *)NULL);

  if(numtasks > 4) //entered at the command line
  {
    cout << "number of tasks is larger than 4, program aborted\n";
    return -1;
  }
/*
  //check usage
  if(argc != 2 ) {
    cout << "USAGE: sfxc01  ctrlFile  or \n"
         << "       sfxc01t ctrlFile  \n\n";
    return -1;
  }
*/  
  //set the control file name
  strcpy(ctrlFile,argv[1]);

  //parse control file for run parameters
  if (RunPrms.parse_ctrlFile(ctrlFile) != 0) {
    cerr << "ERROR: Control file "<< ctrlFile <<", program aborted.\n";
    return -1;
  }
  
  //show version information and control file info
  if (RunPrms.get_messagelvl()> 0)
    cout << "\nSource " << __FILE__ << " compiled at: "
         << __DATE__ << " " <<__TIME__ << "\n\n"
         << "Control file name "  <<  ctrlFile << "\n";
  
  //check control parameters, optionally show them
  if (RunPrms.check_params() != 0) {
    cerr << "ERROR: Run control parameter, program aborted.\n";
    return -1;
  }
  
  if (RunPrms.get_interactive() && RunPrms.get_messagelvl()> 0) askContinue();

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
  
  if (RunPrms.get_interactive() && RunPrms.get_messagelvl()> 0) askContinue();    

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
    if (RunPrms.get_interactive() && RunPrms.get_messagelvl()> 0) askContinue();
  }  

  // NGHK: Has to be a pointer or a reference, 
  //       since Input_reader is an abstract class
  std::vector<Input_reader *> input_readers;
  for (int i=0; i<Nstations; i++) {
    input_readers.push_back(new Input_reader_file(StaPrms[i].get_mk4file()));
  }

  //Find Offsets
  FindOffsets(input_readers, numtasks);

  if ( RunPrms.get_runoption() == 1) {
    //Process data
    //MULTIPLE CORE PROCESSING
    int rank=0;
    cout << "correlation on core " << rank << " started" << endl;
    CorrelateBufs(rank, input_readers);
    cout << "correlation on core " << rank << " finished" << endl;
  }
  return 1;

}

