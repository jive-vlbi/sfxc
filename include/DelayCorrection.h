/*
Author     : RHJ Oerlemans
StartDate  : 20070202
Last change: 20070202
*/

#ifndef DELAYCORRECTION_H
#define DELAYCORRECTION_H

#include <assert.h>

#include <fftw3.h>

#include <math.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

//sfxc includes
#include <types.h>
#include "constPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "InData.h"
#include "delayTable.h"
#include <Data_reader_file.h>
#include <Log_writer.h>


/** Functions and data necessary to do the delay correction
    for each station.**/
class DelayCorrection
{
  public:

    /** Allocate arrays, initialise parameters. **/
    DelayCorrection(GenP &GenPrms, StaP* StaPrms, Log_writer &lg_wrtr);

    /** De-allocate arrays and destroy plans. **/
    ~DelayCorrection();

    /** **/    
    void set_data_reader(int sn, Data_reader *data_reader_); 

    /** Go to desired position in input reader.**/
    void init_reader(int sn, StaP &StaPrms, INT64 startIS); 
    
    /** fills the next segment to be processed by correlator core**/
    void fill_segment();

    /** get the segment filled with delay corrected data. **/
    double** get_segment();

    /** assigns delay table for station number sn**/
    void set_delay_table(int sn, DelayTable &delay_table);

  private:

    //member functions
    // Fill Bufs with delay corrected data.
    void fill_Bufs();
    
    Log_writer& get_log_writer();

    //data members
    Log_writer &log_writer;
    
    INT64  timePtr;     //time in usec wrt 00:00 used for delay table
    double **segm;      //nstation data buffer ready for correlation
    double **Bufs;      //nstations buffers with delay corrected
    double **dcBufs;    //buffers with data for delay correction
    double **dcBufPrev; //previous buffers with data for delay correction
    INT32  BufSize;     //size of one buffer in Bufs
    INT32  BufPtr;      //read pointer in Bufs array
    int    n2fftDC;     //FFT length in delay correction (segment length)
    INT32  Nsegm2DC;    //nr of FFT segments in delay correction
    int    nstations;   //nr of stations
    
    double SR;          //sample rate
    double tbs;         //time between samples
    int    Nf;          //nr of frequencies in frequency scale
    double *fs;         //delta frequency in frequency scale, frequency scale
    double foffset;     //frequency offset in Hertz
    double bwfl;        //band width after filter 
    double startf;      //start frequency
    int    ovrfl;       //oversampling in filter

    int    n2fftcorr;   //FFT length in correlation
    
    fftw_complex *sls;  //FW:in; BW: out
    fftw_complex *spls; //FW:out; BW: in
    fftw_plan    planT2F;//plan for complex FFT Time to Frequeny
    fftw_plan    planF2T;//plan for complex FFT Frequency to Time

    double skyfreq;       //channel sky frequency


    double **data_frame;  //array with data to be delay corrected
    INT32  *df_length;    //data frame length
    INT32  *df_counter;   //data frame counter
    
    vector<DelayTable>    delTbl;
    vector<Data_reader *> data_reader;

};
#endif //DELAYCORRECTION_H
