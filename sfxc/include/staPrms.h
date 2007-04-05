/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Class definitions for Station Parameters
 */

#ifndef STA_PRMS_H
#define STA_PRMS_H

//standard c includes
#include <stdio.h>
#include "Log_writer.h"

const int chmax      =     8; //max nr of channels, nch*fo <=32
const int fomax      =     4; //maximum fan out
const int frameMk4   = 20000; //frame length in bits mk4 file on mk5 disk
const int hdrMk4     =   160; //header length bits  mk4 file on mk5 disk
const int nfrms      =     2; //number of frames to be processed at one go
const int trksMax    =    64; //maximum number of tracks in array

class MPI_Transfer;

class StaP
{
  friend class MPI_Transfer;
  public:

    //default constructor, set default values 
    StaP();

    //destructor
    ~StaP();

    //parse control file for station parameters
    int parse_ctrlFile(char *ctrlFile, int staNr, Log_writer &log_writer);

    //check station parameters
    int check_params(Log_writer &log_writer) const;

    //get functions
    /** The namename of the station. **/
    char* get_stname() const;
    /** The format in which the data is stored. **/
    int   get_datatype() const;
    /** Track bit rate in Mb/s/track. **/
    int   get_tbr() const;
    /** Fanout, can be: 1,2,4 **/
    int   get_fo() const;
    /** bits per sample: mostly 2, 1 sign and 1 magnitude bit **/
    int   get_bps() const;
    /** Tracks per headstack. **/
    int   get_tphs() const;
    /** Number of headstacks: 1 or 2 **/
    int   get_nhs() const;
    /** Fine offset in bytes, determine by trail and error **/
    int   get_boff() const;
    /** Synchronisation track for headstack 1: <32 **/
    int   get_synhs1() const;
    /** Synchronisation track for headstack 2: <32.
     * mandatory if NHS=2, optional if NHS=1 but not relevant
     **/
    int   get_synhs2() const;
    /** Modulation enabled if modulation pattern file is defined. **/
    int   get_mod() const;
    /** Substitute random values for the header **/
    int   get_rndhdr() const;
    /** Filename of the MK4 file**/
    char* get_mk4file() const;
    /** Headermap file. **/
    char* get_hdrmap() const;
    /** Name of the modulation pattern file. **/
    char* get_modpat() const;
    /** Name of the file containing the delay table information. **/
    char* get_delaytable() const;
    /** Name of the file containing the phase table information. **/
    char* get_phasetable() const;
    /** Local oscilator observing station. NGHK: What is this? **/
    INT64 get_loobs() const;
    /** bit shift parameters for sign **/
    const int*  get_signBS() const; //bit shift parameters for sign
    /** bit shift parameters for magn **/
    const int*  get_magnBS() const; //bit shift parameters for magn

  private:
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
    int findMK4data(FILE *ctrlP, Log_writer &log_writer);
    int findDelaydata(FILE *ctrlP, Log_writer &log_writer);
};


#endif // STA_PRMS_H
