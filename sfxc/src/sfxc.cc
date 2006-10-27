/*
Source       sfxc.cc
Title        software FX correlator
Author       RHJ Oerlemans
Started      20060901
Last change  20060912


Description
sfxc correlates the single channel data from N stations recorded on linux
type harddisks. It can run on a multiprocessor machine or cluster.
When sfxc runs on M processors the data from each station is divided into
M chunks of equal length. All chunks from N stations but in the same
time interval are processed on a separate processor. After the correlation
of the time chunks the separate output files can be concatenated.

The diagram below shows how the data from 4 stations is processed
simultaneously on 3 processors. After the correlation the 3 correlator
product files are concatenated into one data file.

        | ST0001 | ST0002 | ST0003 | ST0004 |
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
-

phase table files
-one for each station

Output:
log file
correlator file(s)

Usage:
sfxc  control_file [ro ml nc i] or
sfxct control_file [ro ml nc i]
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

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#include "constVars.h"

//class and function definitions
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "InData.h"
#include "ProcessData.h"


//global variables
//declaration and default settings run parameters
RunP RunPrms;
//declaration and default settings general parameters
GenP GenPrms;


int main(int argc, char *argv[])
{

  //declarations
  char   ctrlFile[lineLength]; // control file name
  int    i, j, Nstations, Ncores, core;
  

  //check usage
  if(argc != 2 ) {
    cout << "USAGE: sfxc  ctrlFile  or \n"
         << "       sfxct ctrlFile  \n\n";
    return -1;
  }
  
  //set the control file name
  strcpy(ctrlFile,argv[1]);

  //parse control file for run parameters
  if (RunPrms.parse_ctrlFile(ctrlFile) != 0) {
    cout << "ERROR: Control file "<< ctrlFile <<", program aborted.\n";
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
  //station parameters class, declaration and default settings
  StaP StaPrms[NstationsMax];

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

  INT64  sliceStartByte[NstationsMax][NcoresMax]; //in bytes
  INT64  sliceStopByte[NstationsMax][NcoresMax]; //frames per core for each station
  //Find Offsets
  FindOffsets(StaPrms, sliceStartByte, sliceStopByte);

  //Process data
  //LATER MULTIPLE CORE PROCESSING, FOR TIME BEING ONLY ONE CORE
  core=0; 
  CorrelateBufs(StaPrms, sliceStartByte, sliceStopByte, core);

}

