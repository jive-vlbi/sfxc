/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef CORRELATIONCORE_H
#define CORRELATIONCORE_H

#include <math.h>

#include <fftw3.h>

#include <types.h>
#include <boost/shared_ptr.hpp>

//c++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

#include "Log_writer.h"
#include <Data_writer.h>
#include <Control_parameters.h>

/**
 * Functions and data necessary to correlate one Time Slice.
 **/
class CorrelationCore
{
public:

  /** Initialise the correlator with proper values, allocate arrays,
      createFFTW plans.**/
  CorrelationCore(Log_writer &log_writer);
  CorrelationCore(Log_writer &log_writer, Correlation_parameters &corr_param);

  /** De-allocate correlator arrays, destroy FFTW plans.**/
  ~CorrelationCore();

  /** Sets the parameters for the correlation **/
  void set_parameters(Correlation_parameters &corr_param);

  /** Initialise array values to zero before the correlation
      of the time slice.**/
  bool init_time_slice();
    
  /** Calculate FFT, Auto and Cross correlations for current segment.**/
  bool correlate_segment(double** in_segm);
    
  /** Average correlation results in the current time slice. **/
  bool average_time_slice();

  /** Write the averaged results for the current time slice.**/
  bool write_time_slice();

  /** **/
  void set_data_writer(boost::shared_ptr<Data_writer> data_writer);

  Data_writer &get_data_writer();

  Log_writer &get_log_writer() {
    return log_writer;
  }
    
private:

  //data members
  boost::shared_ptr<Data_writer> data_writer;

  fftw_complex **accxps; //accumulated fftw_complex accxps[nbslns][];
  double **segm;        //input segments for FFT operation, one per station
  fftw_complex **xps;    //xps: result vectors from FFT
  double *norms;         //normalization coeffs
  
  int   n2fftcorr; //FFT length in correlation
  int   nbslns;    //nr of baselines: autos + cross
  int   nstations; //nr of stations
  int   padding;   //padding factor in FFT

  fftw_plan *p_r2c; //FFT plans

  //function members
  void correlate_baseline(int station1, int station2, int bsln);
  void normalise_correlation(int station1, int station2, int bsln);

  bool cross_polarize;
  int ref_station1, ref_station2;

  Log_writer &log_writer;

  bool parameters_set;
};
#endif // CORRELATIONCORE_H
