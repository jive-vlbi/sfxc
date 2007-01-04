#ifndef STA_PRMS_H
#define STA_PRMS_H

/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Class definitions for Station Parameters

Author     : RHJ Oerlemans
StartDate  : 20060912
Last change: 20061114

*/

//standard c includes
#include <stdio.h>

const int chmax      =     8; //max nr of channels, nch*fo <=32
const int fomax      =     4; //maximum fan out
const int frameMk4   = 20000; //frame length in bits mk4 file on mk5 disk
const int hdrMk4     =   160; //header length bits  mk4 file on mk5 disk
const int nfrms      =     2; //number of frames to be processed at one go
const int trksMax    =    64; //maximum number of tracks in array

class StaP
{

  public:

    //station specific paramters.
    char *stname;  //unique station
    int  datatype;//[MK4|others], now only MK4
    
    //control parameters for Mk4 formatted data file
    int  tbr;      //track bit rate
    int  fo;       //fanout, can be 1,2,4
    int  bps;      //bits per sample. mostly 2, 1 sign and 1 magnitude bit
    int  nhs;      //number of headstacks: 1 or 2
    int  tphs;     //tracks per headstack: fixed at 32
    int  boff;     //fine offset in bytes, determine by trail and error
    int  synhs1;   //synchronisation track for headstack 1: <32
    int  synhs2;   //synchronisation track for headstack 2: <32
    int  mod;      //modulation.  on=1, off=0
    int  rndhdr;   //random header on if 1 and off if 0
    char *mk4file; //input data in mk4 file format
    char *hdrmap;  //headermap file
    char *modpat;  //modulation pattern
    int  signBS[fomax]; //bit shift parameters for sign
    int  magnBS[fomax]; //bit shift parameters for magn
    int  hs;
    int  hm;
    
    //delay correction related data
    char *delaytable; //table with delay data
    char *phasetable; //table with phase
    INT64 loobs;       //local ascilator observing station

    //private functions
    int findMK4data(FILE *ctrlP);
    int findDelaydata(FILE *ctrlP);
    
  public:

    //default constructor, set default values 
    StaP();

    //destructor
    ~StaP();

    //parse control file for station parameters
    int parse_ctrlFile(char *ctrlFile, int staNr);

    //check station parameters
    int check_params();

    //get functions
    char* get_stname();
    int   get_datatype();
    int   get_tbr();
    int   get_fo();
    int   get_bps();
    int   get_tphs();
    int   get_nhs();
    int   get_boff();
    int   get_synhs1();
    int   get_synhs2();
    int   get_mod();
    int   get_rndhdr();
    char* get_mk4file();
    char* get_hdrmap();
    char* get_modpat();
    char* get_delaytable();
    char* get_phasetable();
    INT64 get_loobs();
    int*  get_signBS(); //bit shift parameters for sign
    int*  get_magnBS(); //bit shift parameters for magn

};


#endif // STA_PRMS_H
