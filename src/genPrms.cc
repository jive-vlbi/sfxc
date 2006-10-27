/*
Class function definitions for general parameters

Author     : RHJ Oerlemans
StartDate  : 20060912
Last change: 20060912

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

#include "gen_defines.h"

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//includes for system calls
#include <sys/types.h>
#include <dirent.h>
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
#include "genPrms.h"
#include "genFunctions.h"

//global variables
extern RunP  RunPrms;

//*****************************************************************************
//function definitions
//*****************************************************************************

//get functions
char* GenP::get_experiment(){ return experiment; }
int   GenP::get_yst()       { return yst; }
int   GenP::get_dst()       { return dst; }
int   GenP::get_hst()       { return hst; }
int   GenP::get_mst()       { return mst; }
int   GenP::get_sst()       { return sst; }
int   GenP::get_ysp()       { return ysp; }
int   GenP::get_dsp()       { return dsp; }
int   GenP::get_hsp()       { return hsp; }
int   GenP::get_msp()       { return msp; }
int   GenP::get_ssp()       { return ssp; }
int   GenP::get_nstations() { return nstations; }
char* GenP::get_outdir()    { return outdir;}
char* GenP::get_logfile()   { return logfile;}
char* GenP::get_corfile()   { return corfile;}

int   GenP::get_bwin()      { return bwin;}
int   GenP::get_lsegm()     { return lsegm;}
int   GenP::get_foffset()   { return foffset;}
int   GenP::get_cde()       { return cde;}
int   GenP::get_mde()       { return mde;}
int   GenP::get_rde()       { return rde;}

int   GenP::get_filter()    { return filter;}
int   GenP::get_bwfl()      { return bwfl;}
int   GenP::get_startf()    { return startf;}
int   GenP::get_deltaf()    { return deltaf;}
int   GenP::get_ovrfl()     { return ovrfl;}

int   GenP::get_n2fft()     { return n2fft;}
float GenP::get_ovrlp()     { return ovrlp;}
INT64 GenP::get_nsamp2avg() { return nsamp2avg;}
int   GenP::get_pad()       { return pad;}

INT64 GenP::get_usStart() {

  usStart = dst;                 //days
  usStart = hst +   24* usStart; //hours
  usStart = mst +   60* usStart; //minutes
  usStart = sst +   60* usStart; //minutes
  usStart = 0   + 1000* usStart; //milisecs
  usStart = 0   + 1000* usStart; //microsecs
  return usStart;
  
}  

INT64 GenP::get_usStop() {

  usStop = dsp;                 //days
  usStop = hsp +   24* usStop; //hours
  usStop = msp +   60* usStop; //minutes
  usStop = ssp +   60* usStop; //minutes
  usStop = 0   + 1000* usStop; //milisecs
  usStop = 0   + 1000* usStop; //microsecs
  return usStop;
  
}  

INT64 GenP::get_usEarliest(){ return usEarliest; }
INT64 GenP::get_usLatest()  { return usLatest; }

void  GenP::set_usEarliest(INT64 newEarliest) {
  usEarliest = newEarliest;
}

void  GenP::set_usLatest(INT64 newLatest) {
  usLatest = newLatest;
}

//default constructor, set default values for general control parameters
GenP::GenP()
{
  experiment = new char[256];
  strcpy(experiment,"DefExp");
  yst=dst=hst=mst=sst=0;
  ysp=dsp=hsp=msp=ssp=0;
  nstations = 2;
  outdir  = new char[256];
  outdir = getcwd(NULL, 256);//current working directory
  logfile  = new char[256];
  strcpy(logfile,outdir);
  strcat(logfile,"/DefExp.log");
  corfile  = new char[256];
  strcpy(corfile,outdir);
  strcat(corfile,"/DefExp.cor");

  bwin    = 16000000;
  lsegm   = 2048;
  foffset = 0;
  cde     = 1;
  mde     = 1;
  rde     = 1;

  filter = 0;
  bwfl   = bwin;
  startf = 0;
  deltaf = 1;
  ovrfl  = 1;

  n2fft     = 512;
  ovrlp     = 0.0;
  nsamp2avg = 1600000;
  pad       = 2;
}



//parse control file for general control parameters
int GenP::parse_ctrlFile(char *ctrlFile)
{
  int  retval=0, i, vall;
  FILE *ctrlP;
  char *line, *key, *val, *val1, *val2, *val3, *val4;
  char *logname, *corname;

  //allocate memory
  line = new char[lineLength];
  key  = new char[lineLength];
  val  = new char[lineLength];
  val1 = new char[lineLength];
  val2 = new char[lineLength];
  val3 = new char[lineLength];
  val4 = new char[lineLength];
  logname = new char[lineLength];
  corname = new char[lineLength];

  if( access(ctrlFile, R_OK) != 0 ) {
    cerr << "**** File " << ctrlFile << "is not accessible or does not exist.\n";
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
      
      if (strcmp(key,"EXPERIMENT") == 0) strcpy(experiment,val);
      if (strcmp(key,"OUTDIR") == 0)     strcpy(outdir,val);
      if (strcmp(key,"LOGFILE") == 0)    strcpy(logname,val);
      if (strcmp(key,"CORFILE") == 0)    strcpy(corname,val);
      
      retval = retval + getLongVal(key,val,"NSTATIONS",nstations);
      retval = retval + getLongVal(key,val,"BWIN",bwin);
      retval = retval + getLongVal(key,val,"LSEGM",lsegm);
      retval = retval + getLongVal(key,val,"FOFFSET",foffset);
      retval = retval + getLongVal(key,val,"FILTER",filter);
      retval = retval + getLongVal(key,val,"BWFL",bwfl);
      retval = retval + getLongVal(key,val,"STARTF",startf);
      retval = retval + getLongVal(key,val,"DELTAF",deltaf);
      retval = retval + getLongVal(key,val,"OVRFL",ovrfl);

      retval = retval + getLongVal(key,val,"N2FFT",n2fft);
      retval = retval + getINT64Val(key,val,"NSAMP2AVG",nsamp2avg);
      retval = retval + getFloatVal(key,val,"OVRLP",ovrlp);
      retval = retval + getLongVal(key,val,"PAD",pad);
     
    }

    if (sscanf(line,"%s %s %s %s %s %s\n",key,val,val1,val2,val3,val4) == 6){
      //start time: yyyy ddd hh mm ss
      if (!strcmp(key,"START")) {
        retval = retval + str2int(val,yst);
        retval = retval + str2int(val1,dst);
        retval = retval + str2int(val2,hst);
        retval = retval + str2int(val3,mst);
        retval = retval + str2int(val4,sst);
      }
      //stop time: yyyy ddd hh mm ss
      if (!strcmp(key,"STOP")) {
        retval = retval + str2int(val,ysp);
        retval = retval + str2int(val1,dsp);
        retval = retval + str2int(val2,hsp);
        retval = retval + str2int(val3,msp);
        retval = retval + str2int(val4,ssp);
      }
    }

  }//end while loop

  // close control file
  fclose(ctrlP);

  //release memory
  delete [] line;
  delete [] key;
  delete [] val;
  delete [] val1;
  delete [] val2;
  delete [] val3;
  delete [] val4;

  strcpy(logfile, outdir);
  strcat(logfile,"/");
  strcat(logfile, logname);
  strcpy(corfile, outdir);
  strcat(corfile,"/");
  strcat(corfile, corname);

  return retval;
}



//display and check general parameters
int GenP::check_params()
{

  int retval = 0, i, j, FFTlength, Overlap;
  INT64 tStart, tStop, sec2proc;
  FILE *fl;
  char command[256];

  //calculate some parameters
  //TBD: add year in next calculations
  tStart=dst*24*3600+hst*3600+mst*60+sst;
  tStop =dsp*24*3600+hsp*3600+msp*60+ssp;
  sec2proc=tStop-tStart;
  
  if ( RunPrms.get_messagelvl() > 0) {
    //display general parameters
    cout <<
      endl <<
      "Experiment           = " << experiment << endl <<
      "Start                = " << setw(4) << yst << setw(4) << dst <<
                                   setw(3) << hst << setw(3) << mst <<
                                   setw(3) << sst << " (y d h m s)\n" <<
      "Stop                 = " << setw(4) << ysp << setw(4) << dsp <<
                                   setw(3) << hsp << setw(3) << msp <<
                                   setw(3) << ssp << " (y d h m s)\n" <<
      "Number of stations   = " << nstations << endl <<
      "Output directory     = " << outdir << endl <<
      "Log file             = " << logfile << endl <<
      "Correlator file      = " << corfile << endl << endl <<
      
      "Input bandwidth      = " << bwin << endl <<
      "Segment length delay correction = " << lsegm << endl <<
      "Frequency offset     = " << foffset << endl <<
      "Enabled delay table columns = "
        << cde << " " << mde << " " << rde << " " << endl << endl <<
        
      "Filter enabled if 1  = " << filter << endl <<
      "Filter bandwidth [Hz]= " << bwfl << endl <<
      "Start frequency [Hz] = " << startf << endl <<
      "Frequency resolution = " << deltaf << endl <<
      "Oversampling         = " << ovrfl << endl << endl <<
      
      "Length of segment in correlation = " << n2fft << endl <<
      "Segment overlap      = " << ovrlp << endl <<
      "Number of samples to average = " << nsamp2avg << endl <<
      "Padding with zeros   = " << pad << endl << endl;
  }

  //check control parameters and show faulty ones
  //TBD: add more checks for other paramters
  
  if (dst > 366 || hst > 23 || mst > 59 || sst > 59) {
    cerr << "ERROR: Check start time for values out of range" << endl;
    retval = -1;
  }
  
  if (dsp > 366 || hsp > 23 || msp > 59 || ssp > 59) {
    cerr << "ERROR: Check stop time for values out of range" << endl;
    retval = -1;
  }
  
  if (tStart > tStop) {
    cerr << "ERROR: Start time is later than stop time! \n";
    retval=-1;
  }

  if (sec2proc<1) {
    cerr << "ERROR: Seconds to process is 0, nothing to be done! \n";
    retval=-1;
  }

  fl = fopen(corfile,"w");
  if (!fl) {
    cerr << "ERROR: Cannot create file in directory: " << outdir << endl;
    retval =-1;
  } else {
    fclose(fl);
    //delete empty file
    strcpy(command,"rm -f ");
    strcat(command,corfile);
    system(command);
  }

  if (lsegm > 63) {
    FFTlength = lsegm;
    while (1){
      if (FFTlength%2 != 0) {
        cerr << "ERROR: Length of segment in delay correction = " << lsegm <<
          ", is not a power of 2." << endl;
        retval=-1;
        break;
      } else {
        FFTlength = FFTlength/2;
      }
      if (FFTlength==2) break;
    }
  } else {
    cerr << "ERROR: Length of segment in delay correction = " << lsegm <<
      ", is less than 64." << endl;
    retval=-1;
  }
  
  if (n2fft > 63) {
    FFTlength = n2fft;
    while (1){
      if (FFTlength%2 != 0) {
        cerr << "ERROR: Length of segment in correlation = " << n2fft <<
          ", is not a power of 2." << endl;
        retval=-1;
        break;
      } else {
        FFTlength = FFTlength/2;
      }
      if (FFTlength==2) break;
    }
  } else {
    cerr << "ERROR: Length of segment in correlation = " << n2fft <<
      ", is less than 64." << endl;
    retval=-1;
  }

  Overlap = 100*ovrlp;
  if (!(Overlap==0 || Overlap==25 ||Overlap==50 ||Overlap==75 )) {
    cerr << "ERROR: Overlap = " << ovrlp <<
      ", it should be [0|0.25|0.5|0.75]." << endl;
    retval=-1;
  
  }

  //initialize earliest possible start time and
  //latest possible stop time in micro seconds
  usEarliest = get_usStart();
  usLatest   = get_usStop();
  
  return retval;

}


