/*
Class function definitions for runparameters

Author     : RHJ Oerlemans
StartDate  : 12-09-2006
Last change: 12-09-2006

*/

//these defines have to be the first in source file
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

//enable define on 32 bit CPU, disable on 64 bit CPU
#define THIRTYTWO

//32 bit machine define,
//use open, lseek, off_t in stead off open64, lseek64, off64_t
#ifdef THIRTYTWO
#define _FILE_OFFSET_BITS 64
#endif

//WARNING, check if definitions are appropriate on machine
//definition of 32 bit and 64 bit (un)signed integers
#define INT32  int
#define UINT32 unsigned int
#define INT64  long long
#define UINT64 unsigned long long

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//includes for system calls
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;


//constants
#include "constVars.h"

//the class definitions and function definitions
#include "runPrms.h"
#include "genFunctions.h"


//*****************************************************************************
//function definitions
//*****************************************************************************

//get functions
int RunP::get_messagelvl()  { return messagelvl; }
int RunP::get_interactive() { return interactive; }
int RunP::get_runoption()   { return runoption; }
int RunP::get_ncores()      { return ncores; }



//default constructor, set default values for run parameters
RunP::RunP()
{
  messagelvl  = 0; //only error and abort messages
  interactive = 0; //run automatically
  runoption   = 1; //run program complete
  ncores      = 1; //use one core
}



//parse control file for run parameters
int RunP::parse_ctrlFile(char *ctrlFile)
{
  int  retval=0, i;
  FILE *ctrlP;
  char *line, *key, *val;

  //allocate memory
  line = new char[lineLength];
  key  = new char[lineLength];
  val  = new char[lineLength];

  if( access(ctrlFile, R_OK) != 0 ) {
    cerr << "ERROR: File " << ctrlFile << "is not accessible or does not exist.\n";
    return -1;
  }

  //open control file
  ctrlP = fopen(ctrlFile,"r");

  //read the ctrl file until the end and look for keywords
  while (1) {

    // get next line from file continue or
    // break from while loop at end of file
    if (fgets(line,lineLength,ctrlP) == (char *) NULL) break;

    //split line contents in key and value
    if (sscanf(line,"%s %s\n",key,val) == 2){
      
      retval = retval + getLongVal(key,val,"MESSAGELVL",messagelvl);
      retval = retval + getLongVal(key,val,"INTERACTIVE",interactive);
      retval = retval + getLongVal(key,val,"RUNOPTION",runoption);
      retval = retval + getLongVal(key,val,"NCORES",ncores);
      
    }

  }//end while loop

  // close control file
  fclose(ctrlP);

  //release memory
  delete [] line;
  delete [] key;
  delete [] val;

  return retval;
}



//check the run parameters
int RunP::check_params()
{

  int retval = 0;
  //display run parameters
  if (messagelvl != 0) {
    cout <<
    endl <<
    "Message level        = " << messagelvl << endl <<
    "Interactive          = " << interactive << endl <<
    "Run option           = " << runoption << endl <<
    "Nr of cores          = " << ncores <<
    endl;
  }

  //check run parameters and show faulty ones
  if (!(messagelvl == 0 || messagelvl == 1 || messagelvl == 2 )) {
    cerr <<
    "ERROR: Message level = " << messagelvl << endl <<
    "- 0: only error messages\n" <<
    "- 1: 0 + higher level warning and progress messages\n" <<
    "- 2: 1 + lower level warning and progress messages\n";
    retval=-1;
  }

  if (!(interactive == 0 || interactive == 1)) {
    cerr <<
    "ERROR: Interactive = " << interactive << endl <<
    "- 0: run automatically\n" <<
    "- 1: run interactive\n";
    retval=-1;
  }

  if (!(runoption == 0 || runoption == 1)) {
    cerr <<
    "ERROR: Run option = " << runoption << endl <<
    "- 0: calculate only offsets\n" <<
    "- 1: run complete program";
    retval=-1;
  }

  if ( ncores < 1 ) {
    cerr <<
    "ERROR: Number of cores = " << ncores << ". It should be a positive integer.\n";
    retval=-1;
  }

  return retval;
}


