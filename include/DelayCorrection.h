/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef DELAYCORRECTION_H
#define DELAYCORRECTION_H

#include <assert.h>

#include <complex>
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
#include "Timer.h"
#include "Delay_table_akima.h"
#include <Bits_to_float_converter.h>
#include <Log_writer.h>

#include <Control_parameters.h>


/** Functions and data necessary to do the delay correction
    for each station.**/
class DelayCorrection
{
public:

  /** Allocate arrays, initialise parameters. **/
  DelayCorrection(Log_writer &lg_wrtr);

  DelayCorrection(Correlation_parameters &corr_param, Log_writer &lg_wrtr);

  /** De-allocate arrays and destroy plans. **/
  ~DelayCorrection();

  void set_parameters(Correlation_parameters &corr_param);

  /** **/    
  void set_sample_reader
    (size_t sn, 
     boost::shared_ptr<Bits_to_float_converter> data_reader_); 

  /** Go to desired position in input reader.**/
  bool init_reader(int sn, int64_t startIS); 
  
  /** fills the next segment to be processed by correlator core**/
  bool fill_segment();

  /** get the segment filled with delay corrected data. **/
  double** get_segment();

  /** assigns delay table for station number sn**/
  bool set_delay_table(int sn, Delay_table_akima &delay_table);

  /** Set the start time and the duration of the correlation **/
  void set_start_time(int64_t ms_start);

private:

    //member functions
    // Fill Bufs with delay corrected data.
  bool fill_Bufs();
  bool fill_data_before_delay_correction();
  bool delay_correct();
  bool fractional_bit_shift(double const delay, int const integer_shift);
  bool fringe_stopping(int station, int jsegm);
  
  
  Log_writer& get_log_writer();

  //data members
  Log_writer &log_writer;
  Correlation_parameters corr_param;
  
  int64_t  timePtr;     //time in usec wrt 00:00 used for delay table
  double **segm;      //nstation data buffer ready for correlation
  double **Bufs;      //nstations buffers with delay corrected
  double **dcBufs;    //buffers with data for delay correction
  double **dcBufPrev; //previous buffers with data for delay correction
  int32_t  BufSize;     //size of one buffer in Bufs
  int32_t  BufPtr;      //read pointer in Bufs array
  int    n2fftDC;     //FFT length in delay correction (segment length)
  int32_t  Nsegm2DC;    //nr of FFT segments in delay correction
  int    nstations;   //nr of stations
  
  double SR;          //sample rate
  int    Nf;          //nr of frequencies in frequency scale
  std::vector<double> fs; //delta frequency in frequency scale, frequency scale
  double foffset;     //frequency offset in Hertz
  double bwfl;        //band width after filter 
  double startf;      //start frequency
  int    ovrfl;       //oversampling in filter

  int    n2fftcorr;   //FFT length in correlation
  
  std::vector< std::complex<double> >  sls;      //FW:in; BW: out
  std::vector< std::complex<double> >  sls_freq; //FW:out; BW: in
  fftw_plan    planT2F;//plan for complex FFT Time to Frequeny
  fftw_plan    planF2T;//plan for complex FFT Frequency to Time

  double skyfreq;       //channel sky frequency
  
  std::vector<Delay_table_akima>    delTbl;
  std::vector< boost::shared_ptr<Bits_to_float_converter> > sample_reader;

  int sideband;

  bool parameters_set;

  // For fringe stopping we do a linear approximation
  // maximal_phase_change is the maximal angle between two
  // sample points
  static const double maximal_phase_change = 0.2; // 5.7 degrees
  int n_recompute_delay;
};
#endif //DELAYCORRECTION_H
