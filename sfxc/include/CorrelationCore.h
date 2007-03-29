/*
Author     : RHJ Oerlemans
StartDate  : 20070205
Last change: 20070205

*/

#ifndef CORRELATIONCORE_H
#define CORRELATIONCORE_H

#include <math.h>

#include <fftw3.h>

#include <types.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

#include "Log_writer.h"
#include <Data_writer.h>
#include "genPrms.h"

/**
Functions and data necessary to correlate one Time Slice.
**/
class CorrelationCore
{
  public:

    /** Initialise the correlator with proper values, allocate arrays,
        createFFTW plans.**/
    CorrelationCore(GenP&);

    /** De-allocate correlator arrays, destroy FFTW plans.**/
    ~CorrelationCore();

    /** Initialise array values to zero before the correlation
        of the time slice.**/
    void init_time_slice();
    
    /** Calculate FFT, Auto and Cross correlations for current segment. **/
    void correlate_segment(double** in_segm);
    
    /** Average correlation results in the current time slice. **/
    void average_time_slice();

    /** Write the averaged results for the current time slice.**/
    void write_time_slice();

    /** **/
    void set_data_writer(Data_writer *data_writer);

    Data_writer &get_data_writer();
    
  private:

    //data members
    Data_writer *data_writer;

    fftw_complex **accxps; //accumulated fftw_complex accxps[nbslns][];
    double **segm;        //input segments for FFT operation, one per station
    fftw_complex **xps;    //xps: result vectors from FFT
    double *norms;         //normalization coeffs
  
    int   n2fftcorr; //FFT length in correlation
    int   nbslns;    //nr of baselines: autos + cross
    int   nstations; //nr of stations
    int   padding;   //padding factor in FFT

    fftw_plan *p_r2c; //FFT plans

};
#endif // CORRELATIONCORE_H
