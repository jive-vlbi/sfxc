/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Functions for input data handling
 */

#include <types.h>

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

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
#include "constPrms.h"

//class and function definitions
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "InData.h"

//
extern UINT32 seed;


//*****************************************************************************
//function prototypes only used in this source, not public
//*****************************************************************************

// NGHK: station was sn
int read64datafile(
  //input
  Data_reader & reader,
  //output
  char tracks[][frameMk4*nfrms]);

int read32datafile(
  //input
  Data_reader &reader,
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


Log_writer *log_writer=NULL;

void set_log_writer(Log_writer &log_writer_)
{
  log_writer = &log_writer_;
}

Log_writer &get_log_writer()
{
  assert(log_writer!=NULL);
  return *log_writer;
}



//*****************************************************************************
//show first MK4 header in file
//*****************************************************************************
void show_MK4_header(Data_reader *data_reader, INT64 startIS, 
  StaP &StaPrms, GenP &GenPrms)
{
  if (get_log_writer().get_messagelevel() == 0) return;
  int   jsynch;
  INT64 usTime;

  //find first header in data file
  FindHeaderMk4(*data_reader, jsynch ,usTime, startIS, StaPrms, GenPrms);
  get_log_writer().ask_continue();
}



//*****************************************************************************
//fill Mk4frame if frame counter at end of array
//*****************************************************************************
int fill_Mk4frame(int sn, Data_reader &reader, double **Mk4frame, StaP &StaPrms)
{
  INT64 readstatus=0;
  INT32 r32block[frameMk4];  //32 bit buffer samples of 32 track data
  INT64 r64block[frameMk4];  //64 bit buffer samples of 64 track data
  
  int ifo, jhdr;
  int nhs, fo, rndhdr = 0;
  int sign, magn;
  double smplTBL[2][2];
  INT64 jbuf;
  //work variables
  INT32 work32;
  INT64 work64;

  //initialise lookup table
  smplTBL[0][0]=-7.0;
  smplTBL[0][1]=-2.0;
  smplTBL[1][0]= 2.0;
  smplTBL[1][1]= 7.0;
  
  nhs    = StaPrms.get_nhs();
  fo     = StaPrms.get_fo();
  rndhdr = StaPrms.get_rndhdr();
  
  //extracting for 2 headstacks
  if(nhs==2) {
    readstatus = reader.get_bytes(8*frameMk4, (char*)r64block);
    if (readstatus != 8*frameMk4)
      return -1;//error when reading
      
    for(jbuf=0; jbuf<(frameMk4); jbuf++) {
      work64=r64block[jbuf];
      for(ifo=0; ifo<fo; ifo++) {
        //get sign and magnitude bit for all channels
        sign = (int)((work64 >>  StaPrms.get_signBS()[ifo] ) & 0x1);
        magn = (int)((work64 >>  StaPrms.get_magnBS()[ifo] ) & 0x1);
        //convert sign and magnitude into a double using the lookup table
        Mk4frame[sn][jbuf*fo+ifo] = smplTBL[sign][magn];
      }
    }
  }

  //extracting for 1 headstack
  if(nhs==1) {
    readstatus = reader.get_bytes(4*frameMk4, (char*)r32block);
    if (readstatus != 4*frameMk4) 
      return -1;//error when reading
      
    for(jbuf=0; jbuf<(frameMk4); jbuf++) {
      work32=r32block[jbuf];
      for(ifo=0; ifo<fo; ifo++) {
        //get sign and magnitude bit for all channels
        sign = (int)((work32 >>  StaPrms.get_signBS()[ifo] ) & 0x1);
        magn = (int)((work32 >>  StaPrms.get_magnBS()[ifo] ) & 0x1);
        //convert sign and magnitude into a double using the lookup table
        Mk4frame[sn][jbuf*fo+ifo] = smplTBL[sign][magn];
      }
    }
  }

  //Replace Header with Random Pattern,
  if ( rndhdr ) {
    for(jhdr=0; jhdr<fo*hdrMk4; jhdr++) {
      sign=irbit2(&seed);
      magn=irbit2(&seed);
      Mk4frame[sn][jhdr] = smplTBL[sign][magn];
    }
  }
     
  return readstatus;
}



//*****************************************************************************
// find the header in the Mk4 type file after offset bytes and
// reset usEarliest in GemPrms if necessary
// input : station station number
// output: usTime  header time in us for requested offset
//         jsynch
//*****************************************************************************
int FindHeaderMk4(Data_reader &reader, int& jsynch,
  INT64& usTime, INT64 usStart, StaP &StaPrms, GenP &GenPrms)
{
  int retval = 0;
  
  //buffer for unpacked tracks, NTRACKS tracks, NFRMS Mk4 frames long
  char  tracks[trksMax][frameMk4*nfrms];
  char  hdrmap[strLength];
  INT64 jsynch0, jsynch1;
  int nhs, synhs1, synhs2;

  INT64 day, hr, min, sec;
    
  int Head0,Head1;  //Headstack IDs as seen in the header
  int year0,day0,hh0,mm0,ss0,ms0,us0; //TOT for headstack 0
  int year1,day1,hh1,mm1,ss1,ms1,us1; //TOT for headstack 1
  INT64 TOTusec0, TOTusec1; //in micro seconds
    
  get_log_writer()(1) <<
    "--------------------------------------------------------------------------------\n" <<
    "Start data display for station " << StaPrms.get_stname() << std::endl;
  
  //read and unpack scanfile data into tracks
  nhs = StaPrms.get_nhs();
  if (nhs==1) {
    if (read32datafile(reader, tracks) != 0) {
      get_log_writer().error("error in read32datafile.");
      return -1;
    }
  } else {
    if (read64datafile(reader, tracks) != 0) {
      get_log_writer().error("error in read64datafile.");
      return -1;
    }
  }

  //print track statistics on stdout
  if (get_log_writer().get_messagelevel()> 0)
    printTrackstats(tracks, nhs);
  
  //find sync word(s)
  synhs1 = StaPrms.get_synhs1();
  synhs2 = StaPrms.get_synhs2();
  if (findSyncWord(tracks, synhs1, 0,  &jsynch0) != 0)
    return -1;//no synchronisation word found
  if (nhs==2)
    if(findSyncWord(tracks, synhs2, 32, &jsynch1) != 0)
      return -1;//no synchronisation word found
  jsynch=jsynch0;
  
  strcpy(hdrmap,StaPrms.get_hdrmap());
  if (get_log_writer().get_messagelevel() > 0){
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
      get_log_writer()(0) << "\nWARNING: jsynch mismatch\n";
      return -1;
    }  
    if(TOTusec0 != TOTusec1) {
      get_log_writer()(0) << "\nWARNING: time code mismatch, TOTusec0 = " << TOTusec0
                    << " TOTusec1 = " << TOTusec1 << endl;
      return -1;
    }  
  }

  if (get_log_writer().get_messagelevel()> 0) {
    sec = usStart/1000000;
    min = sec/60;
    sec = sec - min*60;
    hr  = min/60;
    min = min - hr*60;
    day = hr/24;
    hr  = hr - day*24;
    {
      stringstream msg;
      msg << "Requested (slice) start time   = " << endl <<
        setw(4) << GenPrms.get_yst() << "y " << setw(3) << GenPrms.get_dst() << "d " <<
        setw(2) << hr << "h " << setw(2) << min << "m " <<
        setw(2) << sec << "s ";
      get_log_writer().message(1, msg);
    }
  }

  if( usStart < usTime  ) {
    get_log_writer().message(0,
      "Warning  Requested start time is earlier than start time in data file:");
    get_log_writer()(0) << "  * " << usStart << " < " << usTime << std::endl;
  }

  get_log_writer()(1) << "End data display for station " << StaPrms.get_stname() << endl;
                                       
  return retval;
  
}



//*****************************************************************************
// read and unpack data into tracks for 2 headstacks
//*****************************************************************************
int read64datafile(
  //input
  Data_reader & reader,
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
  // read(dataP, rblock, 8*frameMk4*nfrms);
  readstatus= reader.get_bytes(8*frameMk4*nfrms, (char *)rblock);
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
  Data_reader &reader,                   
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
  readstatus= reader.get_bytes(4*frameMk4*nfrms, (char *)rblock);
//   readstatus=read(dataP, rblock, 4*frameMk4*nfrms);
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
  get_log_writer()(0);
  
  get_log_writer() << "Track bit statistics\n";
  get_log_writer() << "_________________________________________________________________";
  for(jtrack=0;jtrack<nolines;jtrack++){
    char buff[80];
    snprintf(buff, 80, "\n| %02d   %f | %02d   %f | %02d   %f | %02d   %f |", 
    jtrack*4+0, trackstats[jtrack*4+0], jtrack*4+1, trackstats[jtrack*4+1],
    jtrack*4+2, trackstats[jtrack*4+2], jtrack*4+3, trackstats[jtrack*4+3]);
    get_log_writer() << buff;
  }
  get_log_writer() << "\n|_______________|_______________|_______________|_______________|";
  
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
  char buff[80];
  char headerfield[6];
  int jtrack;
  INT32 jsample;
  FILE  *hdrP;
        
  //open the header map file
  hdrP=fopen(hdrmap,"r");
  
  //print header content at start of table (stdout)
  if(jsynch0>=64) {
    snprintf(buff, 80, "\n\n            jsynch0=%016lld               ",jsynch0);
    get_log_writer() << buff;
  } else {
    get_log_writer() << "            jsynch0 not found                    ";
  }
  if(nhs==2) {
    if(jsynch1>=64) {
      snprintf(buff,80,"jsynch1=%016lld",jsynch1);
      get_log_writer() << buff;
    } else {
      snprintf(buff, 80, "jsynch1 not found");
      get_log_writer() << buff;
    }
  }

  get_log_writer() << "\n            headstack 0                            ";
  if(nhs==2) {
    get_log_writer() << "headstack 1";
  }
  get_log_writer() << "\n tracks tr1 00000000001111111111222222222233       ";
  if(nhs==2) get_log_writer() << "00000000001111111111222222222233";
  get_log_writer() << "\n        tr0 01234567890123456789012345678901       ";
  if(nhs==2) get_log_writer() << "01234567890123456789012345678901";
  get_log_writer() << "\n";
  get_log_writer() << "\n sample";

  //print table (stdout)
  for(jsample=jsynch0-64;jsample<jsynch0+116;jsample++){
    snprintf(buff,80,"\n %06d     ",jsample);
    get_log_writer() << buff;
    for(jtrack=0;jtrack<32;jtrack++) {
      snprintf(buff,80,"%1d",tracks[jtrack   ][jsample]);
      get_log_writer() << buff;
    }
    fscanf(hdrP,"%s",headerfield);
    snprintf(buff, 80, " %s ",headerfield);
    get_log_writer() << buff;
    if(nhs==2) {
      for(jtrack=0;jtrack<32;jtrack++) {
        snprintf(buff,80,"%1d",tracks[jtrack+32][jsample]);
        get_log_writer() << buff;
      }
    }
  } 
  //print header content at end of table (stdout)
  get_log_writer() << "\n";
  get_log_writer() << "\n sample";
  get_log_writer() << "\n            headstack 0                            ";
  if(nhs==2) get_log_writer() << "headstack 1";
  get_log_writer() << "\n tracks tr1 00000000001111111111222222222233       ";
  if(nhs==2) get_log_writer() << "00000000001111111111222222222233";
  get_log_writer() << "\n        tr0 01234567890123456789012345678901       ";
  if(nhs==2) get_log_writer() << "01234567890123456789012345678901";

  if(jsynch0>=64) {
    snprintf(buff,80,"\n            jsynch0=%016lld               ",jsynch0);
    get_log_writer() << buff;
  } else {
    snprintf(buff,80,"\n\n            jsynch0 not found                    ");
    get_log_writer() << buff; 
    jsynch0=64;
  }
  if(nhs==2) {
    if(jsynch1>=64) {
      snprintf(buff,80,"jsynch1=%016lld",jsynch1);
      get_log_writer() << buff;
    } else {
      get_log_writer() << "jsynch1 not found";
    }
  }
  get_log_writer() << "\n\n";

  fclose(hdrP);
}


//*****************************************************************************
// calculating and printing of time components for the current fram
// results/output
// -TOTusec: total time in usec wrt start of the day 00:00
// -year, day, hh, mm, ss, ms, us: complete time stamp of the frame
//*****************************************************************************
void timeComps(char tracks[][frameMk4*nfrms],int jsynch,int synchtrack,int headS,
  int *Head, int *year, int *day,
  int *hh, int *mm, int *ss, int *ms, int *us,
  INT64 *TOTusec)
{
  // calculating TOT for headstack 
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
  *TOTusec = *hh; 
  *TOTusec = *mm +   60* (*TOTusec); //minutes
  *TOTusec = *ss +   60* (*TOTusec); //minutes
  *TOTusec = *ms + 1000* (*TOTusec); //milisecs
  *TOTusec = *us + 1000* (*TOTusec); //microsecs

  //printing the first found TOT (stdout)
  char buff[80];
  snprintf(buff,80,"TCD on track%2d headstack %1d TOT%1d= ",synchtrack,headS,headS);
  get_log_writer().message(1,buff);
  snprintf(buff,80,"200%1dy %03dd %02dh %02dm %02ds %03d.%03dms\n",
           *year,*day,*hh,*mm,*ss,*ms,*us);
  get_log_writer().message(1,buff);
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


