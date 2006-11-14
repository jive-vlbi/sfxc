/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Class definitions for general parameters

Author     : RHJ Oerlemans
StartDate  : 20060912
Last change: 20061114

*/

#include "gen_defines.h"

class GenP
{

  private:

    //general control parameters
    char  *experiment; //name of the experiment
    int   yst;        //start year
    int   dst;        //start day
    int   hst;        //start hour
    int   mst;        //start minute
    int   sst;        //start second
    int   ysp;        //stop  year
    int   dsp;        //stop  day
    int   hsp;        //stop  hour
    int   msp;        //stop  minute
    int   ssp;        //stop  second
    int   nstations;  //number of radio telescope stations
    char  *outdir;    //output data directory
    char  *logfile;   //log file name
    char  *corfile;   //correlator product file
          
    int   bwin;       //band width input in Hertz
    int   lsegm;      //length of a Fourier segment in a delay correction
    int   foffset;    //frequency offset in Hertz
    int   cde;        //enable CDE column in delay table
    int   mde;        //enable MDE column in delay table
    int   rde;        //enable RDE column in delay table
    
    int   filter;     //enable filtering
    int   bwfl;       //band width filter in Hertz
    int   startf;     //filter start frequency in Hertz
    int   deltaf;     //filter resolution in Hertz
    int   ovrfl;      //enable oversampling

    int   n2fft;      //length of Fourier segment in correlation
    float ovrlp;      //overlap parameter for Fourier segmetns in correlation
    INT64 nsamp2avg;  //number of samples to average
    int   pad;        //padding with zeros in correlation

    INT64 usStart;    //start time in micro seconds without year
    INT64 usStop;     //stop time in micro seconds without year
    INT64 usEarliest; //earliest possible start time in micro seconds
    INT64 usLatest;   //latest possible start time in micro seconds
    
    
  public:

    //default constructor, set default values for Mk4 control parameters
    GenP();

    //parse control file c
    int parse_ctrlFile(char *ctrlFile);

    //check control parameters
    int check_params();

    //get functions
    char* get_experiment();
    int   get_yst();
    int   get_dst();
    int   get_hst();
    int   get_mst();
    int   get_sst();
    int   get_ysp();
    int   get_dsp();
    int   get_hsp();
    int   get_msp();
    int   get_ssp();
    int   get_nstations();
    char* get_outdir();
    char* get_logfile();
    char* get_corfile();
    
    int   get_bwin();
    int   get_lsegm();
    int   get_foffset();
    int   get_cde();
    int   get_mde();
    int   get_rde();

    int   get_filter();
    int   get_bwfl();
    int   get_startf();
    int   get_deltaf();
    int   get_ovrfl();

    int   get_n2fft();
    float get_ovrlp();
    INT64 get_nsamp2avg();
    int   get_pad();

    INT64 get_usStart();
    INT64 get_usStop();
    INT64 get_usEarliest();
    INT64 get_usLatest();

    //set functions
    void  set_usEarliest(INT64);
    void  set_usLatest(INT64);

};


