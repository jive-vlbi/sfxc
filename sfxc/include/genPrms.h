#ifndef GEN_PRMS_H
#define GEN_PRMS_H

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

#include <types.h>
#include <string>

class MPI_Transfer;

class GenP
{
  friend class MPI_Transfer;
  public:

    //default constructor, set default values for Mk4 control parameters
    GenP();

    //parse control file c
    int parse_ctrlFile(char *ctrlFile);

    //check control parameters
    int check_params() const;

    //get functions
    char* get_experiment() const;
    int   get_yst() const;
    int   get_dst() const;
    int   get_hst() const;
    int   get_mst() const;
    int   get_sst() const;
    int   get_ysp() const;
    int   get_dsp() const;
    int   get_hsp() const;
    int   get_msp() const;
    int   get_ssp() const;
    int   get_nstations() const;
    char* get_outdir() const;
    char* get_logfile() const;
    const char* get_corfile() const;
    
    int   get_bwin() const;
    int   get_lsegm() const;
    int   get_foffset() const;
    int   get_cde() const;
    int   get_mde() const;
    int   get_rde() const;

    int   get_filter() const;
    int   get_bwfl() const;
    int   get_startf() const;
    int   get_deltaf() const;
    int   get_ovrfl() const;

    int   get_n2fft() const;
    float get_ovrlp() const;
    INT64 get_nsamp2avg() const;
    int   get_pad() const;

    INT64 get_usStart() const;
    INT64 get_usStop() const;
    INT64 get_usEarliest() const;
    INT64 get_usLatest() const;

    //set functions
    void  set_usEarliest(INT64);
    void  set_usLatest(INT64);

    void set_start(int time[]);
    void set_stop(int time[]);
    void set_corfile(char *filename);

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
    std::string corfile;   //correlator product file

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
};


#endif // GEN_PRMS_H
