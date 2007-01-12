/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Class function definitions for runparameters

Author     : RHJ Oerlemans
StartDate  : 20060912

*/

#include <types.h>

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



//default constructor, set default values for run parameters
RunP::RunP()
{
  messagelvl  = 0; //only error and abort messages
  interactive = 0; //run automatically
  runoption   = 1; //run program complete
}



//parse control file for run parameters
int RunP::parse_ctrlFile(char *ctrlFile)
{
  int  retval=0;
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
int RunP::check_params() const
{

  int retval = 0;
  //display run parameters
  if (messagelvl > 0) {
    cout <<
    "--------------------------------------------------------------------------------\n" <<
    "General run settings\n" <<
    endl <<
    "Message level        = " << messagelvl << endl <<
    "Interactive          = " << interactive << endl <<
    "Run option           = " << runoption << endl <<
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

  return retval;
}


