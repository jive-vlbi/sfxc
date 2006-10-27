/*
Functions for input data handling

Author     : RHJ Oerlemans
StartDate  : 20061005
Last change: 20061005

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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "gen_defines.h"

//constants
#include "constVars.h"

//class and function definitions
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "InData.h"

//global variables
extern RunP  RunPrms;
extern GenP  GenPrms;

//*****************************************************************************
//function prototypes only used in this source, not public
//*****************************************************************************

int read64datafile(
  //input
  int dataP, 
  //output
  char tracks[][frameMk4*nfrms]);

int read32datafile(
  //input
  int dataP, 
  //output
  char tracks[][frameMk4*nfrms]);

void printTrackstats(char tracks[][frameMk4*nfrms], int nhs);

int findSyncWord(
  //input
  char tracks[][frameMk4*nfrms], INT32 synchtrack, int headS,
  //output
  INT64 *jsynch);

void printFrameHeader(
  //input
  char tracks[][frameMk4*nfrms], INT64 jsynch0, INT64 jsynch1, int nhs,
  char *hdrmap);

void timeComps(char tracks[][frameMk4*nfrms],int jsynch,int synchtrack,int headS,
  int *Head, int *year, int *day,
  int *hh, int *mm, int *ss, int *ms, int *us,
  INT64 *TOTusec);

int fHead(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS);

int fyear(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS);

int fday(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS);

int fhh(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS);

int fmm(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS);

int fss(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS);

int fms(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS);



//*****************************************************************************
//find offsets in bytes in data files based on requested start time:
//- Ncores offsets have to be found in each data file
//- Ncores dBytes have to be found which is the length in
//  bytes to be processed by a core
//*****************************************************************************
int FindOffsets(StaP StaPrms[], INT64 sliceStartByte[][NcoresMax],
  INT64 sliceStopByte[][NcoresMax])
{
  
  int   retval = 0, i, j, sn, NrStations, cn, Ncores;
  int   jsynch[NstationsMax];
  INT64 usTime[NstationsMax], offTime[NstationsMax];
  INT64 offFrames[NstationsMax], offBytes[NstationsMax];
  INT64 StartByte[NstationsMax], offSynch[NstationsMax];
  INT64 NFrames[NstationsMax], FrPcore[NstationsMax];
  INT64 deltaBytes[NstationsMax];

  NrStations = GenPrms.get_nstations();
  Ncores = RunPrms.get_ncores();
  
  //find first headers in data files, reset usEarliest if necessary,
  //return usTime and jsynch for requested byte offset
  for (sn=0; sn<NrStations; sn++) {
    if (StaPrms[sn].get_datatype() == Mk4) {
      FindHeaderMk4(StaPrms, sn, StaPrms[sn].get_boff(), jsynch[sn], usTime[sn]);
    }
    if (RunPrms.get_interactive() && RunPrms.get_messagelvl()> 0) askContinue();
  }
  
  //calculate start offsets in bytes for all data files
  for (sn=0; sn<NrStations; sn++) {
    if (StaPrms[sn].get_datatype() == Mk4) {
      offTime[sn] = GenPrms.get_usEarliest() - usTime[sn];
      offFrames[sn] = offTime[sn] * StaPrms[sn].get_tbr()/frameMk4;
      if (StaPrms[sn].get_nhs() == 1) {
        offBytes[sn] = offFrames[sn] * frameMk4 * 4;
        offSynch[sn] = (jsynch[sn] -64) *4;
      } else {
        offBytes[sn] = offFrames[sn] * frameMk4 * 8;
        offSynch[sn] = (jsynch[sn] -64) *8;
      }
      StartByte[sn] = StaPrms[sn].get_boff() + offBytes[sn] + offSynch[sn];
      cout << "usTime    =" << usTime[sn] << endl;
      cout << "offTime   =" << offTime[sn] << endl;
      cout << "offFrames =" << offFrames[sn] << endl;
      cout << "offBytes  =" << offBytes[sn] << endl;
      cout << "offSynch  =" << offSynch[sn] << endl;
      cout << "StartByte =" << StartByte[sn] << endl;
    }
  }
  
  //find headers for StartByte in data files, only done for monitoring
  if ( RunPrms.get_messagelvl()> 0) {
    for (sn=0; sn<NrStations; sn++) {
      if (StaPrms[sn].get_datatype() == Mk4) {
        FindHeaderMk4(StaPrms, sn, StartByte[sn], jsynch[sn], usTime[sn]);
      }
      if ( RunPrms.get_interactive() ) askContinue();
    }
  }

  //for all stations check if requested stoptime is in the data file
  //if not reset usLatest 
  for (sn=0; sn<NrStations; sn++) {
    if (StaPrms[sn].get_datatype() == Mk4) {
      checkStoptime(StaPrms, sn, StartByte[sn]);
    }
  }

  for (sn=0; sn<NrStations; sn++) {
    if (StaPrms[sn].get_datatype() == Mk4) {
      //number off frames in available data range
      NFrames[sn] =
      (GenPrms.get_usLatest() - GenPrms.get_usEarliest()) * StaPrms[sn].get_tbr()/frameMk4;
      //Frames to be processed per core
      FrPcore[sn] = NFrames[sn]/Ncores;
      cout << "FrPcore[" << sn << "]=" << FrPcore[sn] <<endl;
      //delta
      if (StaPrms[sn].get_nhs() == 1) {    
        deltaBytes[sn] = FrPcore[sn]*frameMk4*4;
      } else {
        deltaBytes[sn] = FrPcore[sn]*frameMk4*8;
      }
      cout << "deltaBytes[" << sn << "]=" << deltaBytes[sn] <<endl;
    }
  }
  
  
  //for all core and all stations set offsets
  for (sn=0; sn<NrStations; sn++) {
    cout << "station=" << sn <<" start stop: ";
    for (cn=0; cn<Ncores; cn++) {
      sliceStartByte[sn][cn] = StartByte[sn] + cn*deltaBytes[sn];
      sliceStopByte[sn][cn]  = sliceStartByte[sn][cn] + deltaBytes[sn];
      cout << sliceStartByte[sn][cn] << " " << sliceStopByte[sn][cn] << "   ";
    }
    cout << endl;
  }
  return retval;
  
}


//*****************************************************************************
//check if the stop time is in the data file reset usLatest if necessary
//*****************************************************************************
int checkStoptime(StaP StaPrms[], int sn, INT64 StartByte) {

  int retval=0;
  INT64 deltaTime, deltaFrames, deltaBytes, StopByte;
  struct stat dataFile;

  deltaTime = GenPrms.get_usLatest() - GenPrms.get_usEarliest();
  deltaFrames = deltaTime * StaPrms[sn].get_tbr()/frameMk4;
  if ( StaPrms[sn].get_nhs() == 1)
    deltaBytes = deltaFrames*frameMk4*4;
  else
    deltaBytes = deltaFrames*frameMk4*8;
        
  StopByte = StartByte + deltaBytes;
  //StopByte should be smaller than file length
  stat(StaPrms[sn].get_mk4file(),&dataFile);
  if (StopByte > dataFile.st_size) {
    //recalculate the StopByte
    StopByte = dataFile.st_size;
    deltaBytes = StopByte - StartByte;
    //deltaBytes should be an integer nr of frames
    if ( StaPrms[sn].get_nhs() == 1){
      deltaFrames = deltaBytes/(frameMk4*4);
    } else {
      deltaFrames = deltaBytes/(frameMk4*8);
    }
    deltaTime = deltaFrames/StaPrms[sn].get_tbr()*frameMk4;
    //reset latest possible stop time
    GenPrms.set_usLatest(GenPrms.get_usEarliest() + deltaTime);
    if( RunPrms.get_messagelvl()> 0)
      cout << endl <<
      "WARNING: Requested stop time is later than latest time in data file.\n" <<
      "         Stop time is reset to earlier possible value.\n\n";
  }
  
  return retval;
  
}



//*****************************************************************************
// find the header in the Mk4 type file after offset bytes and
// reset usEarliest in GemPrms if necessary
// input : sn      station number
//         StaPrms station parameters
//         offset  in bytes from where to start reading the file
// output: usTime  header time in us for requested offset
//         jsynch
//*****************************************************************************
int FindHeaderMk4(StaP StaPrms[], int sn, INT64 offset, int& jsynch, INT64& usTime)
{

  int retval = 0;
  
  int dataP;
  //buffer for unpacked tracks, NTRACKS tracks, NFRMS Mk4 frames long
  char  tracks[trksMax][frameMk4*nfrms];
  char  hdrmap[strLength];
  off_t offsetstatus;  //return value of lseek64
  int read_ok;
  INT64 jsynch0, jsynch1;
  int nhs, synhs1, synhs2;
  
  INT32 jsec;
  
  int Head0,Head1;  //Headstack IDs as seen in the header
  int year0,day0,hh0,mm0,ss0,ms0,us0; //TOT for headstack 0
  int year1,day1,hh1,mm1,ss1,ms1,us1; //TOT for headstack 1
  INT64 TOTusec0, TOTusec1, usStart; //in micru seconds
  
  //open the data file
  dataP = open(StaPrms[sn].get_mk4file(),O_RDONLY,0);
  
  if (RunPrms.get_messagelvl()> 0) {
    cout << "For station " << StaPrms[sn].get_stname() << endl;
    cout << "Look for header in data file: " << StaPrms[sn].get_mk4file() << endl;
  }  
  
  offsetstatus=lseek(dataP,offset,SEEK_SET);
  if (offsetstatus != offset) {
    cerr << "Could not go to requested offset position! " <<  offset << endl;
    cerr << "Offsetstatus = " << offsetstatus << endl;
    return -1;
  }
  
  //read and unpack scanfile data into tracks
  nhs = StaPrms[sn].get_nhs();
  if (nhs==1) {
    if (read32datafile(dataP, tracks) != 0) {
      cerr << "Error in read32datafile.\n";
      return -1;
    }
  } else {
    if (read64datafile(dataP, tracks) != 0) {
      cerr << "Error in read64datafile.\n";
      return -1;
    }
  }

  //print track statistic on stdout
  if (RunPrms.get_messagelvl()> 0)
    printTrackstats(tracks, nhs);
  
  //find sync word(s)
  synhs1 = StaPrms[sn].get_synhs1();
  synhs2 = StaPrms[sn].get_synhs2();
  if (findSyncWord(tracks, synhs1, 0,  &jsynch0) != 0)
    return -1;//no synchronisation word found
  if (nhs==2)
    if(findSyncWord(tracks, synhs2, 32, &jsynch1) != 0)
      return -1;//no synchronisation word found
  jsynch=jsynch0;
  
  strcpy(hdrmap,StaPrms[sn].get_hdrmap());
  if (RunPrms.get_messagelvl()> 0){
    //printFrameHeader
    printFrameHeader(tracks, jsynch0, jsynch1, nhs, hdrmap);
  }  

      
  // calculating TOT for headstack 0 
  timeComps(tracks, jsynch0, nhs, 0,
    &Head0, &year0, &day0, &hh0, &mm0, &ss0, &ms0, &us0, &TOTusec0);
    
  //set time for output in microseconds
  usTime = TOTusec0;
  
  if (nhs==2) {
    // calculating TOT for headstack 1
    timeComps(tracks, jsynch1, synhs2, 1,
      &Head1, &year1, &day1, &hh1, &mm1, &ss1, &ms1, &us1, &TOTusec1);
      
    if(jsynch0 != jsynch1) {
      cerr << "\nWARNING: jsynch mismatch\n";
      return -1;
    }  
    if(TOTusec0 != TOTusec1) {
      cerr << "\nWARNING: time code mismatch, TOTusec0 = " << TOTusec0
           << " TOTusec1 = " << TOTusec1 << endl;
      return -1;
    }  
  }

  if (RunPrms.get_messagelvl()> 0) {
    cout << "Requested start time           = " <<
    setw(4) << GenPrms.get_yst() << "y " << setw(3) << GenPrms.get_dst() << "d " <<
    setw(2) << GenPrms.get_hst() << "h " << setw(2) << GenPrms.get_mst() << "m " <<
    setw(2) << GenPrms.get_sst() << "s " << endl;
  }

  usStart = GenPrms.get_usStart();
  
  if( usStart-usTime <0 ) {
    GenPrms.set_usEarliest(usTime);
    if( RunPrms.get_messagelvl()> 0)
      cout << endl <<
      "WARNING: Requested start time is earlier than earliest time in data file.\n" <<
      "         Start time is reset to later possible value.\n\n";
  }


  //close the data file
  close(dataP);
                                       
  return retval;
  
}



//*****************************************************************************
// read and unpack data into tracks for 2 headstacks
//*****************************************************************************
int read64datafile(
  //input
  int dataP, 
  //output
  char tracks[][frameMk4*nfrms])
{
  int jbuf1,jtrack;
  INT64 readstatus;
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // next parameters have to be 64 bit
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  INT64 rblock[frameMk4*nfrms], work;
  //read from data file into [frameMk4*nfrms] nr of blocks,
  //[EIGHT*frameMk4*nfrms] nr of bytes
  readstatus=read(dataP, rblock, 8*frameMk4*nfrms);
  if( (readstatus =! 8*frameMk4*nfrms ) )
    return -1;
  
  //Unpack data into track samples
  for(jbuf1=0; jbuf1<frameMk4*nfrms; jbuf1++){
    work=rblock[jbuf1];
    //Unpack Mark5 data into tracks
    for(jtrack=0; jtrack<64; jtrack++){
      tracks[jtrack][jbuf1]=(work >> jtrack) & 0x1;
    }
  }
  return 0;
}


//*****************************************************************************
// read and unpack data into tracks for one headstack
//*****************************************************************************
int read32datafile(
  //input
  int dataP, 
  //output
  char tracks[][frameMk4*nfrms])
{
  int jbuf1,jtrack;
  INT32 readstatus;
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // next parameters have to be 32 bit
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  INT32 rblock[frameMk4*nfrms], work;
  //read from data file into [frameMk4*nfrms] nr of blocks,
  //[FOUR*frameMk4*nfrms] nr of bytes
  readstatus=read(dataP, rblock, 4*frameMk4*nfrms);
  if( (readstatus =! 4*frameMk4*nfrms) )
    return -1;
  
  //Unpack data into track samples
  for(jbuf1=0; jbuf1<frameMk4*nfrms; jbuf1++){
    work=rblock[jbuf1];
    //Unpack Mark5 data into tracks
    for(jtrack=0; jtrack<32; jtrack++){
      tracks[jtrack][jbuf1]=(work >> jtrack) & 0x1;
    }
  }
  return 0;
}





//*****************************************************************************
// print and calculate the track statistics
//*****************************************************************************
void printTrackstats(char tracks[][frameMk4*nfrms], int nhs)
{
  float trackstats[64];
  int jtrack;
  INT32 jsample;
  int   notracks, nolines;

  if(nhs==1) {
    notracks=32;
    nolines=8;
  } else {
    notracks=64;
    nolines=16;
  } 

  //initialize trackstats
  for(jtrack=0;jtrack<notracks;jtrack++) trackstats[jtrack]=0.0;
  //Calculate track statistics
  for (jsample=0; jsample<(frameMk4*nfrms); jsample++){
    for( jtrack=0; jtrack<notracks; jtrack++){
      trackstats[jtrack]=trackstats[jtrack]+tracks[jtrack][jsample];
    }
  } 
  for(jtrack=0; jtrack<notracks; jtrack++){
    trackstats[jtrack]=trackstats[jtrack]/(frameMk4*nfrms);
  }

  //report track statistics (stdout)
  printf("\nTrack bit statistics\n");
  printf(
  "\n_________________________________________________________________");
  for(jtrack=0;jtrack<nolines;jtrack++){
    printf("\n| %02d   %f | %02d   %f | %02d   %f | %02d   %f |", 
    jtrack*4+0, trackstats[jtrack*4+0], jtrack*4+1, trackstats[jtrack*4+1],
    jtrack*4+2, trackstats[jtrack*4+2], jtrack*4+3, trackstats[jtrack*4+3]);
  }
  printf(
  "\n|_______________|_______________|_______________|_______________|");
  printf("\n\n");
}



//*****************************************************************************
//find synchronisation word
//*****************************************************************************
int findSyncWord(
  //input
  char tracks[][frameMk4*nfrms], INT32 synchtrack, int headS,
  //output
  INT64 *jsynch)
{
  int   synchbuff[frameMk4*nfrms],synchcorr[frameMk4*nfrms];
  INT32 jsample, jcorr;
  int   retval = 0;
  
  //calculate synchbuffers on headstack
  for (jsample=0;jsample<frameMk4*nfrms;jsample++){
    synchbuff[jsample]=tracks[synchtrack + headS ][jsample];
  }
  for(jsample=0;jsample<(frameMk4*nfrms-64);jsample++){
    synchcorr[jsample]=0;
    for(jcorr=0;jcorr<32;jcorr++){
      synchcorr[jsample]=synchcorr[jsample]+synchbuff[jsample+jcorr];
    }
  }
          
  *jsynch=0; 
  for(jsample=0;jsample<frameMk4*nfrms;jsample++){
    if( (synchcorr[jsample] ==32) && (*jsynch<64) ) {
      *jsynch=jsample;
    } 
  }

  if(*jsynch == 0){
    cerr << "No synchronisation word found!\n";
    retval=-1;
  }
  return retval;
  
}


//*****************************************************************************
// print contents of Frame Header
//*****************************************************************************
void printFrameHeader(
  //input
  char tracks[][frameMk4*nfrms], INT64 jsynch0, INT64 jsynch1, int nhs,
  char *hdrmap)
{
  char headerfield[6];
  int jtrack;
  INT32 jsample;
  FILE  *hdrP;
        
  //open the header map file
  hdrP=fopen(hdrmap,"r");
  
  //print header content at start of table (stdout)
  if(jsynch0>=64) {
    printf("\n\n            jsynch0=%016lld               ",jsynch0);
  } else {
    printf("\n\n            jsynch0 not found                    ");
  }
  if(nhs==2) {
    if(jsynch1>=64) {
      printf("jsynch1=%016lld",jsynch1);
    } else {
      printf("jsynch1 not found");
    }
  }

  printf("\n            headstack 0                            ");
  if(nhs==2) printf("headstack 1");
  printf("\n tracks tr1 00000000001111111111222222222233       ");
  if(nhs==2) printf("00000000001111111111222222222233");
  printf("\n        tr0 01234567890123456789012345678901       ");
  if(nhs==2) printf("01234567890123456789012345678901");
  printf("\n");
  printf("\n sample");

  //print table (stdout)
  for(jsample=jsynch0-64;jsample<jsynch0+116;jsample++){
    printf("\n %06d     ",jsample);
    for(jtrack=0;jtrack<32;jtrack++)
      printf("%1d",tracks[jtrack   ][jsample]);
    fscanf(hdrP,"%s",headerfield);
    printf(" %s ",headerfield);
    if(nhs==2) {
      for(jtrack=0;jtrack<32;jtrack++)
        printf("%1d",tracks[jtrack+32][jsample]);
    }
  } 
  //print header content at end of table (stdout)
  printf("\n");
  printf("\n sample");
  printf("\n            headstack 0                            ");
  if(nhs==2) printf("headstack 1");
  printf("\n tracks tr1 00000000001111111111222222222233       ");
  if(nhs==2) printf("00000000001111111111222222222233");
  printf("\n        tr0 01234567890123456789012345678901       ");
  if(nhs==2) printf("01234567890123456789012345678901");

  if(jsynch0>=64) {
    printf(  "\n            jsynch0=%016lld               ",jsynch0);
  } else {
    printf("\n\n            jsynch0 not found                    ");
    jsynch0=64;
  }
  if(nhs==2) {
    if(jsynch1>=64) {
      printf("jsynch1=%016lld",jsynch1);
    } else {
      printf("jsynch1 not found");
    }
  }
  printf("\n\n");

  fclose(hdrP);
}


//*****************************************************************************
// calculating and printing of time components
//*****************************************************************************
void timeComps(char tracks[][frameMk4*nfrms],int jsynch,int synchtrack,int headS,
  int *Head, int *year, int *day,
  int *hh, int *mm, int *ss, int *ms, int *us,
  INT64 *TOTusec)
{
  // calculating TOT for headstack 0
  *Head = fHead(tracks,synchtrack,jsynch,headS*32);
  *year = fyear(tracks,synchtrack,jsynch,headS*32);
  *day  = fday (tracks,synchtrack,jsynch,headS*32);
  *hh   = fhh  (tracks,synchtrack,jsynch,headS*32);
  *mm   = fmm  (tracks,synchtrack,jsynch,headS*32);
  *ss   = fss  (tracks,synchtrack,jsynch,headS*32);
  *ms   = fms  (tracks,synchtrack,jsynch,headS*32);
  *us   = *ms%5;
  *us   = *us*250; // see Mk4 memo on format definition
  //total number of microseconds
  //*TOTusec=(*us + 1000*(*ms+1000*(*ss+60*(*mm+60*(*hh+24* *day) ) ) ) );
  //WARNING: do not try equation above this line.
  //ohterwise large integers will not be calculated correctly
  //Use next lines to calculate
  *TOTusec = *day;                 //days
  *TOTusec = *hh +   24* (*TOTusec); //hours
  *TOTusec = *mm +   60* (*TOTusec); //minutes
  *TOTusec = *ss +   60* (*TOTusec); //minutes
  *TOTusec = *ms + 1000* (*TOTusec); //milisecs
  *TOTusec = *us + 1000* (*TOTusec); //microsecs

  if (RunPrms.get_messagelvl()> 0) {
    //printing the first found TOT (stdout)
    printf("TCD on track%2d headstack %1d TOT%1d= ",synchtrack,headS,headS);
    printf("200%1dy %03dd %02dh %02dm %02ds %03d.%03dms\n",
      *year,*day,*hh,*mm,*ss,*ms,*us);
  }

}    



//*****************************************************************************
//  fHead
//*****************************************************************************
int fHead(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS)
{
  return
    tracks[headS+syntrk][jsync-31] + 2*tracks[headS+syntrk][jsync-32];
}


//*****************************************************************************
//  fyear
//*****************************************************************************
int fyear(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS)
{
  return
  (
    tracks[syntrk+headS][jsync+32+ 0]*8 +
    tracks[syntrk+headS][jsync+32+ 1]*4 +
    tracks[syntrk+headS][jsync+32+ 2]*2 +
    tracks[syntrk+headS][jsync+32+ 3]
  )*1;


}



//*****************************************************************************
//  fday
//*****************************************************************************
int fday(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS)
{
  return
  (
    tracks[syntrk+headS][jsync+32+ 4]*8 +
    tracks[syntrk+headS][jsync+32+ 5]*4 +
    tracks[syntrk+headS][jsync+32+ 6]*2 +
    tracks[syntrk+headS][jsync+32+ 7]
  )*100
  +
  (
    tracks[syntrk+headS][jsync+32+ 8]*8 +
    tracks[syntrk+headS][jsync+32+ 9]*4 +
    tracks[syntrk+headS][jsync+32+10]*2 +
    tracks[syntrk+headS][jsync+32+11]
  )*10
  +
  (
    tracks[syntrk+headS][jsync+32+12]*8 +
    tracks[syntrk+headS][jsync+32+13]*4 +
    tracks[syntrk+headS][jsync+32+14]*2 +
    tracks[syntrk+headS][jsync+32+15]
  )*1;
}




//*****************************************************************************
//  fhh
//*****************************************************************************
int fhh(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS)
{
  return
  (
    tracks[syntrk+headS][jsync+32+16]*8 +
    tracks[syntrk+headS][jsync+32+17]*4 +
    tracks[syntrk+headS][jsync+32+18]*2 +
    tracks[syntrk+headS][jsync+32+19])*10
  +
  (
    tracks[syntrk+headS][jsync+32+20]*8 +
    tracks[syntrk+headS][jsync+32+21]*4 +
    tracks[syntrk+headS][jsync+32+22]*2 +
    tracks[syntrk+headS][jsync+32+23]
  )*1;
}  




//*****************************************************************************
//  fmm
//*****************************************************************************
int fmm(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS)
{
  return
  (
    tracks[syntrk+headS][jsync+32+24]*8 +
    tracks[syntrk+headS][jsync+32+25]*4 +
    tracks[syntrk+headS][jsync+32+26]*2 +
    tracks[syntrk+headS][jsync+32+27]
  )*10
  +
  (
    tracks[syntrk+headS][jsync+32+28]*8 +
    tracks[syntrk+headS][jsync+32+29]*4 +
    tracks[syntrk+headS][jsync+32+30]*2 +
    tracks[syntrk+headS][jsync+32+31]
  )*1;

}



//*****************************************************************************
//  fss
//*****************************************************************************
int fss(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS)
{
  return
  (
    tracks[syntrk+headS][jsync+32+32]*8 +
    tracks[syntrk+headS][jsync+32+33]*4 +
    tracks[syntrk+headS][jsync+32+34]*2 +
    tracks[syntrk+headS][jsync+32+35]
  )*10
  +
  (
    tracks[syntrk+headS][jsync+32+36]*8 +
    tracks[syntrk+headS][jsync+32+37]*4 +
    tracks[syntrk+headS][jsync+32+38]*2 +
    tracks[syntrk+headS][jsync+32+39]
  )*1;


}



//*****************************************************************************
//  fms
//*****************************************************************************
int fms(char tracks[][frameMk4*nfrms], INT32 syntrk, INT64 jsync, int headS)
{
  return
  (
    tracks[syntrk+headS][jsync+32+40]*8 +
    tracks[syntrk+headS][jsync+32+41]*4 +
    tracks[syntrk+headS][jsync+32+42]*2 +
    tracks[syntrk+headS][jsync+32+43]
  )*100
  +
  (
    tracks[syntrk+headS][jsync+32+44]*8 +
    tracks[syntrk+headS][jsync+32+45]*4 +
    tracks[syntrk+headS][jsync+32+46]*2 +
    tracks[syntrk+headS][jsync+32+47]
  )*10
  +
  (
    tracks[syntrk+headS][jsync+32+48]*8 +
    tracks[syntrk+headS][jsync+32+49]*4 +
    tracks[syntrk+headS][jsync+32+50]*2 +
    tracks[syntrk+headS][jsync+32+51]
  )*1;
}


