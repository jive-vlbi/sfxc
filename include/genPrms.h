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
#include "Log_writer.h"

class MPI_Transfer;

class GenP
{
  friend class MPI_Transfer;
  public:

    //default constructor, set default values for Mk4 control parameters
    GenP();

    //parse control file c
    int parse_ctrlFile(char *ctrlFile, Log_writer&log_writer);

    //check control parameters
    int check_params(Log_writer &log_writer) const;

    //get functions
    
    /** Name of the experiment. **/
    char* get_experiment() const;
    /** year in the starttime. **/
    int   get_yst() const;
    /** day in the starttime. **/
    int   get_dst() const;
    /** hour in the starttime. **/
    int   get_hst() const;
    /** minutes in the starttime. **/
    int   get_mst() const;
    /** seconds in the starttime. **/
    int   get_sst() const;
    /** year in the stoptime. **/
    int   get_ysp() const;
    /** day in the stoptime. **/
    int   get_dsp() const;
    /** hour in the stoptime. **/
    int   get_hsp() const;
    /** minutes in the stoptime. **/
    int   get_msp() const;
    /** seconds in the stoptime. **/
    int   get_ssp() const;
    
    /** The number of stations **/
    int   get_nstations() const;
    /** Directory to put the correlated files **/
    char* get_outdir() const;
    /** Name of the logfile. **/
    char* get_logfile() const;
    /** Name of the correlation output file. **/
    const char* get_corfile() const;
    
    /** Input band width in Hz. **/
    int   get_bwin() const;
    /** Length of a Fourier segment in delay correction. **/
    int   get_lsegm() const;
    /** Frequency offset. **/
    int   get_foffset() const;
    /** Enable the CDE column in the delay table. **/
    int   get_cde() const;
    /** Enable the MDE column in the delay table. **/
    int   get_mde() const;
    /** Enable the RDE column in the delay table. **/
    int   get_rde() const;

    /** Filter data (settings the same for all stations)
        01-09-2006: filter not implemented in sfxc. **/
    int   get_filter() const;
    /** Bandwidth after optional filtering, Default BWFL=BWIN if FILTER=0. **/
    int   get_bwfl() const;
    /** Filter start frequency, Default STARTF=0 if FITLER=0. **/
    int   get_startf() const;
    /** Filter resolution in Hz. **/
    int   get_deltaf() const;
    /** Filter oversampling. Default OVRFL=1 if FILTER=0 **/
    int   get_ovrfl() const;

    /** Length of Fourier segment in correlation. **/
    int   get_n2fft() const;
    /** overlap parameter for Fourier segments in correlation.
     * 0: no overlap. 
     **/
    float get_ovrlp() const;
    /** Number of samples to average in correlation. **/
    INT64 get_nsamp2avg() const;
    /** Padding with zeros in correlation.
     * NGHK: Values? 
     **/
    int   get_pad() const;

    /** Get the start time in microseconds from the beginning of the day. **/
    INT64 get_usStart() const;
    /** Get the stop time in microseconds from the beginning of the day.  **/
    INT64 get_usStop() const;
    /** Get the earliest possible time for correlation 
     * in microseconds from the beginning of the day. **/
    INT64 get_usEarliest() const;
    /** Depricated: do not use. **/
    INT64 get_usLatest() const;

    //set functions
    /** Set the earliest possible time for correlation 
     * in microseconds from the beginning of the day. **/
    void  set_usStart(INT64);
    void  set_usStop(INT64);
    void  set_usEarliest(INT64);
    /** Depricated: do not use. **/
    void  set_usLatest(INT64);

    /** Set the start time in microseconds from the beginning of the day. **/
    void set_start(int time[]);
    /** Set the stop time in microseconds from the beginning of the day. **/
    void set_stop(int time[]);
    /** Set the name of the correlation output file. **/
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
