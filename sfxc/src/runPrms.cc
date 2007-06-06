/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Class function definitions for runparameters
 */

#include <types.h>
#include <assert.h>

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
#include "constPrms.h"

//the class definitions and function definitions
#include "runPrms.h"
#include "genFunctions.h"


//*****************************************************************************
//function definitions
//*****************************************************************************

//get functions
int RunP::get_messagelvl()  const { return messagelvl; }
int RunP::get_interactive() const { return interactive; }
int RunP::get_runoption()   const { return runoption; }
int RunP::get_ref_station(int i) const { 
  if (i == 0) return ref_station1; 
  assert(i == 1);
  return ref_station2;
}



//default constructor, set default values for run parameters
RunP::RunP()
{
  messagelvl  = 0; //only error and abort messages
  interactive = 0; //run automatically
  runoption   = 1; //run program complete
  ref_station1 = -1;//correlate all possible base lines (auto and crosses)
  ref_station2 = -1;//correlate all possible base lines (auto and crosses)
}



//parse control file for run parameters
int RunP::parse_ctrlFile(char *ctrlFile, Log_writer &log_writer)
{
  int  retval=0;
  FILE *ctrlP;
  char *line, *key, *val;

  //allocate memory
  line = new char[lineLength];
  key  = new char[lineLength];
  val  = new char[lineLength];

  if( access(ctrlFile, R_OK) != 0 ) {
    cerr << "ERROR: File " << ctrlFile << " is not accessible or does not exist.\n";
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
      
      retval = retval + getLongVal(key,val,"MESSAGELVL",messagelvl, log_writer);
      retval = retval + getLongVal(key,val,"INTERACTIVE",interactive, log_writer);
      retval = retval + getLongVal(key,val,"RUNOPTION",runoption, log_writer);
      retval = retval + getLongVal(key,val,"REFSTATION1",ref_station1, log_writer);      
      retval = retval + getLongVal(key,val,"REFSTATION2",ref_station2, log_writer);      
      
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
int RunP::check_params(Log_writer &log_writer) const
{

  int retval = 0;
  //display run parameters
  log_writer.set_current_messagelevel(1);
  log_writer <<
    "--------------------------------------------------------------------------------\n";
  log_writer <<
    "General run settings\n" << endl;
  log_writer <<
    "Message level        = ";
  log_writer << messagelvl;
  log_writer << endl;
  log_writer <<
    "Interactive          = " << interactive << endl;
  log_writer <<
    "Run option           = " << runoption << endl;
  log_writer <<
    "Reference station1   = " << ref_station1 << endl;
  log_writer <<
    "Reference station2   = " << ref_station2 << endl;
  log_writer <<
    endl;

  //check run parameters and show faulty ones
  if (!(messagelvl == 0 || messagelvl == 1 || messagelvl == 2 )) {
    log_writer(0) <<
    "ERROR: Message level = " << messagelvl << endl <<
    "- 0: only error messages\n" <<
    "- 1: 0 + higher level warning and progress messages\n" <<
    "- 2: 1 + lower level warning and progress messages\n";
    retval=-1;
  }

  if (!(interactive == 0 || interactive == 1)) {
    log_writer(0) <<
    "ERROR: Interactive = " << interactive << endl <<
    "- 0: run automatically\n" <<
    "- 1: run interactive\n";
    retval=-1;
  }

  if (!(runoption == 0 || runoption == 1)) {
    log_writer(0) <<
    "ERROR: Run option = " << runoption << endl <<
    "- 0: calculate only offsets\n" <<
    "- 1: run complete program\n";
    retval=-1;
  }

  return retval;
}


