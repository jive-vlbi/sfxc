/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Class function definitions for general parameters

Author     : RHJ Oerlemans
StartDate  : 20060912
Last change: 20060912

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

// NGHK: remove this, for now to have access to the Log_writer  
#include <ProcessData.h>


//constants
#include "constPrms.h"

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
char* GenP::get_experiment() const { return experiment; }
int   GenP::get_yst()        const { return yst; }
int   GenP::get_dst()        const { return dst; }
int   GenP::get_hst()        const { return hst; }
int   GenP::get_mst()        const { return mst; }
int   GenP::get_sst()        const { return sst; }
int   GenP::get_ysp()        const { return ysp; }
int   GenP::get_dsp()        const { return dsp; }
int   GenP::get_hsp()        const { return hsp; }
int   GenP::get_msp()        const { return msp; }
int   GenP::get_ssp()        const { return ssp; }
int   GenP::get_nstations()  const { return nstations; }
char* GenP::get_outdir()     const { return outdir;}
char* GenP::get_logfile()    const { return logfile;}
const char* GenP::get_corfile()    const { return corfile.c_str();}

int   GenP::get_bwin()       const { return bwin;}
int   GenP::get_lsegm()      const { return lsegm;}
int   GenP::get_foffset()    const { return foffset;}
int   GenP::get_cde()        const { return cde;}
int   GenP::get_mde()        const { return mde;}
int   GenP::get_rde()        const { return rde;}

int   GenP::get_filter()     const { return filter;}
int   GenP::get_bwfl()       const { return bwfl;}
int   GenP::get_startf()     const { return startf;}
int   GenP::get_deltaf()     const { return deltaf;}
int   GenP::get_ovrfl()      const { return ovrfl;}

int   GenP::get_n2fft()      const { return n2fft;}
float GenP::get_ovrlp()      const { return ovrlp;}
INT64 GenP::get_nsamp2avg()  const { return nsamp2avg;}
int   GenP::get_pad()        const { return pad;}

INT64 GenP::get_usStart()  const {
  return usStart;
  
}  

INT64 GenP::get_usStop()  const {
  return usStop;
}  

INT64 GenP::get_usEarliest() const { return usEarliest; }
INT64 GenP::get_usLatest()   const { 
  return usLatest; 
}

void GenP::set_usStart(INT64 start) {
  usStart = start;
}
void GenP::set_usStop(INT64 stop) {
  usStop = stop;
}

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
  nsamp2avg = 1600000;
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
    log_writer(0) << "**** File " << ctrlFile << "is not accessible or does not exist.\n";
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
      
      retval = retval + getLongVal(key,val,"NSTATIONS",nstations, log_writer);
      retval = retval + getLongVal(key,val,"BWIN",bwin, log_writer);
      retval = retval + getLongVal(key,val,"LSEGM",lsegm, log_writer);
      retval = retval + getLongVal(key,val,"FOFFSET",foffset, log_writer);
      retval = retval + getLongVal(key,val,"FILTER",filter, log_writer);
      retval = retval + getLongVal(key,val,"BWFL",bwfl, log_writer);
      retval = retval + getLongVal(key,val,"STARTF",startf, log_writer);
      retval = retval + getLongVal(key,val,"DELTAF",deltaf, log_writer);
      retval = retval + getLongVal(key,val,"OVRFL",ovrfl, log_writer);

      retval = retval + getLongVal(key,val,"N2FFT",n2fft, log_writer);
      retval = retval + getINT64Val(key,val,"NSAMP2AVG",nsamp2avg, log_writer);
      retval = retval + getFloatVal(key,val,"OVRLP",ovrlp, log_writer);
      retval = retval + getLongVal(key,val,"PAD",pad, log_writer);
     
    }

    if (sscanf(line,"%s %s %s %s %s %s\n",key,val,val1,val2,val3,val4) == 6){
      //start time: yyyy ddd hh mm ss
      if (!strcmp(key,"START")) {
        int time[5];
        retval = retval + str2int(val,time[0]);
        retval = retval + str2int(val1,time[1]);
        retval = retval + str2int(val2,time[2]);
        retval = retval + str2int(val3,time[3]);
        retval = retval + str2int(val4,time[4]);
        set_start(time);
      }
      //stop time: yyyy ddd hh mm ss
      if (!strcmp(key,"STOP")) {
        int time[5];
        retval = retval + str2int(val,time[0]);
        retval = retval + str2int(val1,time[1]);
        retval = retval + str2int(val2,time[2]);
        retval = retval + str2int(val3,time[3]);
        retval = retval + str2int(val4,time[4]);
        set_stop(time);
      }
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
  
  //initialize earliest possible start time and
  //latest possible stop time in micro seconds
  usEarliest = get_usStart();
  usLatest   = get_usStop();
  
  return retval;
}



//display and check general parameters
int GenP::check_params(Log_writer &log_writer) const
{

  int retval = 0, FFTlength, Overlap;
  INT64 tStart, tStop, sec2proc;
  FILE *fl;
  char command[256];

  //calculate some parameters
  //TBD: add year in next calculations
  tStart=dst*24*3600+hst*3600+mst*60+sst;
  tStop =dsp*24*3600+hsp*3600+msp*60+ssp;
  sec2proc=tStop-tStart;

  //display general parameters
  stringstream msg;
  msg <<
  "--------------------------------------------------------------------------------\n" <<
  "General experiment related control parameters.\n" <<
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
  log_writer.message(1,msg);


  //check control parameters and show faulty ones
  //TBD: add more checks for other paramters
  
  if (dst > 366 || hst > 23 || mst > 59 || sst > 59) {
    log_writer(0) << "ERROR: Check start time for values out of range" << endl;
    retval = -1;
  }
  
  if (dsp > 366 || hsp > 23 || msp > 59 || ssp > 59) {
    log_writer(0) << "ERROR: Check stop time for values out of range" << endl;
    retval = -1;
  }
  
  if (tStart > tStop) {
    log_writer(0) << "ERROR: Start time is later than stop time! \n";
    retval=-1;
  }

  if (sec2proc<1) {
    log_writer(0) << "ERROR: Seconds to process is 0, nothing to be done! \n";
    retval=-1;
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

  Overlap = (int) (100*ovrlp);
  if (!(Overlap==0 || Overlap==25 ||Overlap==50 ||Overlap==75 )) {
    log_writer(0) << "ERROR: Overlap = " << ovrlp <<
      ", it should be [0|0.25|0.5|0.75]." << endl;
    retval=-1;
  
  }

  return retval;

}


void GenP::set_start(int time[]) {
  yst = time[0];
  dst = time[1];
  hst = time[2];
  mst = time[3];
  sst = time[4];
  
  usStart = hst;                 //hours
  usStart = mst +   60* usStart; //minutes
  usStart = sst +   60* usStart; //minutes
  usStart = 0   + 1000* usStart; //milisecs
  usStart = 0   + 1000* usStart; //microsecs
}

void GenP::set_stop(int time[]) {
  ysp = time[0];
  dsp = time[1];
  hsp = time[2];
  msp = time[3];
  ssp = time[4];

  usStop = hsp;                //hours
  usStop = msp +   60* usStop; //minutes
  usStop = ssp +   60* usStop; //minutes
  usStop = 0   + 1000* usStop; //milisecs
  usStop = 0   + 1000* usStop; //microsecs
}
void GenP::set_corfile(char *filename) {
  corfile = filename;
}
