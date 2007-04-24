/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Class function definitions for general parameters
 */

#include <types.h>

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
#include "constPrms.h"

//the class definitions and function definitions
#include "genPrms.h"
#include "genFunctions.h"


//*****************************************************************************
//function definitions
//*****************************************************************************

//get functions
char* GenP::get_experiment() const { return experiment; }

int   GenP::get_yst()        const { return yst; }
int   GenP::get_dst()        const { return dst; }
int   GenP::get_hst()        const { return hst; }
int   GenP::get_mst()        const { return mst; }
int   GenP::get_sst()        const { return sst; }

int   GenP::get_duration()   const { return duration; }
INT64 GenP::get_usDur()      const { return duration*1000000; }

int   GenP::get_nstations()  const { return nstations; }
char* GenP::get_outdir()     const { return outdir;}
char* GenP::get_logfile()    const { return logfile;}
const char* GenP::get_corfile()    const { return corfile.c_str();}

double GenP::get_skyfreq()    const { return skyfreq;}
double GenP::get_bwin()       const { return bwin;}
int    GenP::get_lsegm()      const { return lsegm;}
double GenP::get_foffset()    const { return foffset;}
int    GenP::get_cde()        const { return cde;}
int    GenP::get_mde()        const { return mde;}
int    GenP::get_rde()        const { return rde;}

int    GenP::get_filter()     const { return filter;}
double GenP::get_bwfl()       const { return bwfl;}
double GenP::get_startf()     const { return startf;}
double GenP::get_deltaf()     const { return deltaf;}
int    GenP::get_ovrfl()      const { return ovrfl;}

int   GenP::get_n2fft()      const { return n2fft;}
float GenP::get_ovrlp()      const { return ovrlp;}
float GenP::get_time2avg()   const { return time2avg;}
INT64 GenP::get_usTime2Avg() const { return (INT64)(time2avg*1000000);}
int   GenP::get_pad()        const { return pad;}


INT64 GenP::get_usStart()  const {  return usStart;}

//TODO RHJO: 20070213 check which of the functions until DEPRICATED? is depricated
//INT64 GenP::get_usStop()   const {  return usStop;}

void GenP::set_usStart(INT64 start) { usStart = start; }
void GenP::set_duration(int dur) { duration = dur; }
//void GenP::set_usStop(INT64 stop)   { usStop = stop;}

//INT64 GenP::get_usEarliest() const { return usEarliest; }
//INT64 GenP::get_usLatest()   const { return usLatest;}

//void  GenP::set_usEarliest(INT64 newEarliest) { usEarliest = newEarliest;}
//void  GenP::set_usLatest(INT64 newLatest)     { usLatest = newLatest;}
// DEPRICATED?

//default constructor, set default values for general control parameters
GenP::GenP()
{
  experiment = new char[256];
  strcpy(experiment,"DefExp");
  yst=dst=hst=mst=sst=0;
  duration=0;
  nstations = 0;
  outdir  = new char[256];
  outdir = getcwd(NULL, 256);//current working directory
  logfile  = new char[256];
  strcpy(logfile,outdir);
  strcat(logfile,"/DefExp.log");
  corfile = outdir;
  corfile.append("/DefExp.cor");

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
  time2avg  = 0.5;
  pad       = 2;
}


//parse control file for general control parameters
int GenP::parse_ctrlFile(char *ctrlFile, Log_writer&log_writer)
{
  int  retval=0;
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
    log_writer(0) << "**** File " << ctrlFile
                  << "is not accessible or does not exist.\n";
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
      if (strcmp(key,"START") == 0)      set_start(val);
      
      retval = retval + getLongVal(key,val,"DURATION",duration, log_writer);
      retval = retval + getLongVal(key,val,"NSTATIONS",nstations, log_writer);
      retval = retval + getDoubleVal(key,val,"BWIN",bwin, log_writer);
      retval = retval + getLongVal(key,val,"N2FFTDEL",lsegm, log_writer);
      retval = retval + getDoubleVal(key,val,"SKYFREQ",skyfreq, log_writer);
      retval = retval + getLongVal(key,val,"FILTER",filter, log_writer);
      retval = retval + getDoubleVal(key,val,"BWFL",bwfl, log_writer);
      retval = retval + getDoubleVal(key,val,"STARTF",startf, log_writer);
      retval = retval + getDoubleVal(key,val,"DELTAF",deltaf, log_writer);
      retval = retval + getLongVal(key,val,"OVRFL",ovrfl, log_writer);

      retval = retval + getLongVal(key,val,"N2FFTCORR",n2fft, log_writer);
      retval = retval + getFloatVal(key,val,"TIME2AVG",time2avg, log_writer);
      retval = retval + getFloatVal(key,val,"OVRLP",ovrlp, log_writer);
      retval = retval + getLongVal(key,val,"PAD",pad, log_writer);
     
    }

    if (sscanf(line,"%s %s %s %s\n",key,val,val1,val2) == 4){
      if (!strcmp(key,"DELCOLS")) {
        retval = retval + str2int(val,cde);
        retval = retval + str2int(val1,mde);
        retval = retval + str2int(val2,rde);
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
  corfile = outdir;
  corfile.append("/");
  corfile.append(corname);

  // NGHK: From check_params until the end:
  if (!filter) {
    bwfl = bwin;
    startf = 0;
    deltaf = 0;
    ovrfl = 1;
  }

  //TODO RHJO check if depricated
  //initialize earliest possible start time and
  //latest possible stop time in micro seconds
//  usEarliest = get_usStart();
//  usLatest   = get_usStop();
  
  return retval;
}



//display and check general parameters
int GenP::check_params(Log_writer &log_writer) const
{

  int retval = 0, FFTlength, Overlap;
  FILE *fl;
  char command[256];

  //display general parameters
  stringstream msg;
  msg <<
  "--------------------------------------------------------------------------------\n" <<
  "General experiment related control parameters.\n" <<
  endl <<
  "Experiment           = " << experiment << endl <<
  "Start                = " << setw(4) << yst << setw(4) << dst <<
                               setw(3) << hst << setw(3) << mst <<
                               setw(3) << sst << setw(4) << milisst << " (y d h m s m)\n" <<
  "Duration             = " << duration << endl <<                             
  "Number of stations   = " << nstations << endl <<
  "Output directory     = " << outdir << endl <<
  "Log file             = " << logfile << endl <<
  "Correlator file      = " << corfile << endl << endl <<
   
  "Sky frequency        = " << skyfreq << endl <<
  "Input bandwidth      = " << bwin << endl <<
  "Segment length delay correction = " << lsegm << endl <<
  "Enabled delay table columns = "
    << cde << " " << mde << " " << rde << " " << endl << endl <<
      
  "Filter enabled if 1  = " << filter << endl <<
  "Filter bandwidth [Hz]= " << bwfl << endl <<
  "Start frequency [Hz] = " << startf << endl <<
  "Frequency resolution = " << deltaf << endl <<
  "Oversampling         = " << ovrfl << endl << endl <<
    
  "Length of segment in correlation = " << n2fft << endl <<
  "Segment overlap      = " << ovrlp << endl <<
  "Time to average (sec)= " << time2avg << endl <<
  "Padding with zeros   = " << pad << endl << endl;
  log_writer.message(1,msg);


  //check control parameters and show faulty ones
  //TBD: add more checks for other paramters
  
  if (dst > 366 || hst > 23 || mst > 59 || sst > 59) {
    log_writer(0) << "ERROR: Check start time for values out of range" << endl;
    retval = -1;
  }
  
  fl = fopen(corfile.c_str(),"w");
  if (!fl) {
    log_writer(0) << "ERROR: Cannot create file in directory: " << outdir << endl;
    retval =-1;
  } else {
    fclose(fl);
    //delete empty file
    strcpy(command,"rm -f ");
    strcat(command,corfile.c_str());
    system(command);
  }

  if (lsegm > 63) {
    FFTlength = lsegm;
    while (1){
      if (FFTlength%2 != 0) {
        log_writer(0) << "ERROR: Length of segment in delay correction = " << lsegm <<
          ", is not a power of 2." << endl;
        retval=-1;
        break;
      } else {
        FFTlength = FFTlength/2;
      }
      if (FFTlength==2) break;
    }
  } else {
    log_writer(0) << "ERROR: Length of segment in delay correction = " << lsegm <<
      ", is less than 64." << endl;
    retval=-1;
  }
  
  if (n2fft > 63) {
    FFTlength = n2fft;
    while (1){
      if (FFTlength%2 != 0) {
        log_writer(0) << "ERROR: Length of segment in correlation = " << n2fft <<
          ", is not a power of 2." << endl;
        retval=-1;
        break;
      } else {
        FFTlength = FFTlength/2;
      }
      if (FFTlength==2) break;
    }
  } else {
    log_writer(0) << "ERROR: Length of segment in correlation = " << n2fft <<
      ", is less than 64." << endl;
    retval=-1;
  }

  if ( pad < 1 || pad > 2) {//TODO RHJO check if these are the only valid values
    log_writer(0) << "ERROR: Padding should be 1 or 2 \n";
    retval=-1;
  }
  
  Overlap = (int) (100*ovrlp);
  if (!(Overlap==0 || Overlap==25 ||Overlap==50 ||Overlap==75 )) {
    log_writer(0) << "ERROR: Overlap = " << ovrlp <<
      ", it should be [0|0.25|0.5|0.75]." << endl;
    retval=-1;
  
  }

  return retval;

}

//extract start time values from string and 
//calculate start wrt 00:00 in micro seconds
void GenP::set_start(string Time) {
  yst     = str_to_long(Time,0,4);  //pos=0, length=4
  dst     = str_to_long(Time,5,3);
  hst     = str_to_long(Time,9,2);
  mst     = str_to_long(Time,12,2);
  sst     = str_to_long(Time,15,2);
  milisst = str_to_long(Time,18,3);
  
  usStart = hst;                      //hours
  usStart = mst      +   60* usStart; //minutes
  usStart = sst      +   60* usStart; //minutes
  usStart = milisst  + 1000* usStart; //milisecs
  usStart = 0        + 1000* usStart; //microsecs
}


void GenP::set_corfile(char *filename) {
  corfile = filename;
}
