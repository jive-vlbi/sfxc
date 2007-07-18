/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <utils.h>
#include "DelayCorrection.h"

//Allocate arrays, initialise parameters
DelayCorrection::DelayCorrection(Log_writer &lg_wrtr)
  : log_writer(lg_wrtr), parameters_set(false)
{
}
//Allocate arrays, initialise parameters
DelayCorrection::DelayCorrection(GenP &GenPrms_, 
                                 StaP *StaPrms_, 
                                 Log_writer &lg_wrtr)
  : log_writer(lg_wrtr), parameters_set(false)
{
  set_parameters(GenPrms_, StaPrms_);
}

//Allocate arrays, initialise parameters
void DelayCorrection::set_parameters(GenP &GenPrms, StaP *StaPrms_)
{
  parameters_set = true;

  StaPrms     = StaPrms_;

  nstations   = GenPrms.get_nstations();
  n2fftDC     = GenPrms.get_lsegm();
  SR          = 2.0*GenPrms.get_bwfl()*GenPrms.get_ovrfl();
  BufSize     = BufTime * (int)(SR/1000000.0);
  tbs         = 1.0/SR;
  Nsegm2DC    = BufSize/n2fftDC;

  sideband    = GenPrms.get_sideband();

  Nf          = n2fftDC/2+1; //number of frequencies
  double dfr  = 1.0/(n2fftDC*tbs); // delta frequency
  fs.resize(Nf); // frequency array

  for (int jf=0; jf<Nf; jf++) {
    //frequency scale in the segment
    fs[jf]=sideband*(jf*dfr-0.5*GenPrms.get_bwfl()-GenPrms.get_foffset());
  }

  foffset     = GenPrms.get_foffset();
  bwfl        = GenPrms.get_bwfl();
  ovrfl       = GenPrms.get_ovrfl();
  startf      = GenPrms.get_startf();
  skyfreq     = GenPrms.get_skyfreq();

  n2fftcorr   = GenPrms.get_n2fft();
  
  segm = new double*[nstations];
  Bufs = new double*[nstations];
  dcBufs = new double*[nstations];
  dcBufPrev =new double*[nstations];
  for (int sn=0; sn<nstations; sn++){
    segm[sn] = new double[n2fftcorr];
    Bufs[sn] = new double[BufSize];
    dcBufs[sn] = new double[3*BufSize];
    dcBufPrev[sn] = new double[2*BufSize];
  }
  
  timePtr = GenPrms.get_usStart();//set timePtr to start for delay
  BufPtr = BufSize;//set read pointer to end of Bufs, because Bufs not filled
  
  //arrays and plans for delay correction
  sls.resize(n2fftDC);
  sls_freq.resize(n2fftDC);
  planT2F = fftw_plan_dft_1d(n2fftDC, 
			     (fftw_complex *)&sls[0], (fftw_complex *)&sls_freq[0], 
			     FFTW_BACKWARD, FFTW_ESTIMATE);
  planF2T = fftw_plan_dft_1d(n2fftDC, 
			     (fftw_complex *)&sls_freq[0], (fftw_complex *)&sls[0], 
			     FFTW_FORWARD,  FFTW_ESTIMATE);
  //TODO RHJO: ask SP why not use fftw_plan_r2c_1d and fftw_plan_c2r_1d. Try!
  //4b) and 4c) probably not necessary anymore


  //set vector sizes
  delTbl.resize(nstations);//delay tables 
  sample_reader.resize(nstations);//sample_readers
}


//De-allocate arrays and destroy plans
DelayCorrection::~DelayCorrection()
{

  for (int sn=0; sn<nstations; sn++){
    delete [] segm[sn];
    delete [] Bufs[sn];
    delete [] dcBufs[sn];
  }
  delete [] segm;  
  delete [] Bufs;
  delete [] dcBufs;

  fftw_destroy_plan(planF2T);
  fftw_destroy_plan(planT2F);
  
}



//set local data reader parameter
void DelayCorrection::set_sample_reader
  (int sn, boost::shared_ptr<Bits_to_float_converter> sample_reader_)
{
  sample_reader[sn]=sample_reader_;
}

void DelayCorrection::set_start_time(int64_t us_start) {
  timePtr = us_start;//set timePtr to start for delay
}


//go to desired position in input reader for station sn
bool DelayCorrection::init_reader(int sn, int64_t startIS)
{
  assert(sample_reader[sn] != 
         boost::shared_ptr<Bits_to_float_converter>());
  BufPtr = BufSize;//set read pointer to end of Bufs, because Bufs not filled

  //initialise dcBufPrev with data from input channel (can be Mk4 file)
  int bytes_to_read = 2*BufSize;
  int bytes_read = 0;
  while (bytes_read != bytes_to_read) {
    int status = sample_reader[sn]->get_data(bytes_to_read-bytes_read,
                                             &dcBufPrev[sn][bytes_read]);
    if (status <= 0) {
      assert(false);
      return false;
    }
    bytes_read += status;
  }
  assert(bytes_read == bytes_to_read);
  return true;
}


// fills the next segment to be processed by correlator core.
bool DelayCorrection::fill_segment()
{
  //(re)fill Bufs when all data in Bufs is processed
  if ( (BufPtr + n2fftcorr) > BufSize ) {
    if (!fill_Bufs()) return false;
    timePtr=timePtr+BufTime;
    BufPtr=0;
  }
  //fill segm using delay corrected data in Bufs
  for (int i = 0; i < nstations; i++){
    for (int j = 0; j < n2fftcorr; j++){
      segm[i][j] = Bufs[i][j+ BufPtr];
    }
  }
  BufPtr=BufPtr+n2fftcorr;

  return true;
}


// returns pointer to segment with delay corrected data.
double **DelayCorrection::get_segment()
{
  return segm;
}

bool DelayCorrection::delay_correct() {
  assert(parameters_set);
  int64_t  Time; //time in micro seconds
  double Cdel_start, Cdel_end;
  int jshift; //address shift due to signal delay wrt Earth center
  double time_of_one_correlation_segment = n2fftDC*tbs*1000000;

  for (int stations=0; stations<nstations; stations++){
    //apply delay and phase corrections for all segments (n2fftDC long)
    //in other words process data in dcBufs, output in Bufs
    Cdel_start = delTbl[stations].calcDelay(timePtr, DelayTable::Cdel);
    for (int jsegm=0; jsegm<Nsegm2DC; jsegm++) {
      // micro sec 
      Time = timePtr + (int64_t)(jsegm*(time_of_one_correlation_segment)); 
      Cdel_end = delTbl[stations].calcDelay(Time, DelayTable::Cdel);

      // 1)calculate the address shift due to time delay for the current segment
      jshift = (int)(Cdel_start/tbs+0.5);

      int32_t offset = 2*BufSize + jshift + jsegm*n2fftDC;
      assert(offset >= 0);
      assert(offset <= 3*BufSize-n2fftDC);
      // 2)apply the address shift when filling the complex sls array
      for (int jl=0; jl<n2fftDC; jl++){
        sls[jl].real() = dcBufs[stations][offset + jl];
        sls[jl].imag() = 0.0;
      }

      // Take the average for the fractional bit shift
      fractional_bit_shift((Cdel_start+Cdel_end)/2, jshift);

      fringe_stopping(stations, jsegm);
      
      Cdel_start = Cdel_end;
    }
    
    //fill dcBufsPrev with part 2 and 3 from dcBufs.
    //in other words: remember for filling the next Bufs
    memcpy(&dcBufPrev[stations][0], 
           &dcBufs[stations][BufSize], 
           2*BufSize*sizeof(double));

  }

  return true;
}

bool DelayCorrection::fill_data_before_delay_correction() {
  for (int station=0; station<nstations; station++) {
    //fill part 1 and 2 of dcBufs with data from dcBufPrev
    memcpy(&dcBufs[station][0], &dcBufPrev[station][0], 2*BufSize*sizeof(double));
    
    int bytes_to_read = BufSize;
    int bytes_read = 0, status = 1;
    while ((status > 0) && (bytes_read != bytes_to_read)) {
      status = 
        sample_reader[station]->get_data(bytes_to_read-bytes_read,
	   		                                 &dcBufs[station][2*BufSize]+bytes_read);
      bytes_read += status;
    }

    if (bytes_read != bytes_to_read) {
      std::cout << "status != bytes_to_read, with station = " << station 
                << std::endl;
      return false;
    }
  }
  return true;
}

bool DelayCorrection::fractional_bit_shift(double const delay,
					   int const integer_shift) {
  // 3) execute the complex to complex FFT, from Time to Frequency domain
  //    input: sls. output sls_freq
  fftw_execute(planT2F);
      
  // 4a)apply normalization : 
  // not needed will be cancelled by the normalisation of the autocorrelation
  //   for (int jl=0; jl<n2fftDC/2+1; jl++){
  //     sls_freq[jl] /= n2fftDC;
  //   }
      
  // 4b)multiply element 0 and n2fftDC/2 by 0.5
  //    to avoid jumps at segment borders

  sls_freq[0] *= 0.5;
  sls_freq[n2fftDC/2] *= 0.5;//Nyquist
      
  // 4c) zero the unused subband (?)
  for (int jl=n2fftDC/2+1;jl<n2fftDC;jl++){
    sls_freq[jl] = 0.0;
  }

  // 5a)calculate the fract bit shift (=phase corrections in freq domain)
  double dfs  = delay/tbs - integer_shift;

  double tmp1 = -2.0*M_PI*dfs*tbs;
  double tmp2 = 0.5*M_PI*integer_shift/ovrfl;
  // 5b)apply phase correction in frequency range
  for (int jf = 0; jf < Nf; jf++){
    //phi  = -2.0*M_PI*dfs*tbs*fs[jf] + 0.5*M_PI*integer_shift/ovrfl;
    double phi  = tmp1*fs[jf] + tmp2;
    std::complex<double> tmp(cos(phi),sin(phi));
    sls_freq[jf] *= tmp;
  }

  // 6a)execute the complex to complex FFT, from Frequency to Time domain
  //    input: sls_freq. output sls
  fftw_execute(planF2T);
  return true;
}

bool DelayCorrection::fringe_stopping(int station, int jsegm) {
  // FYI: phi_end-phi is approximately .3 for 256 lags

  // Optimized: compute a delay after n_recompute samples
  const int n_recompute_delay = (256 < n2fftDC ? 256 : n2fftDC);

  int64_t time = timePtr + (int64_t)(jsegm*n2fftDC*tbs*1000000);
  int64_t delta_time = (int64_t)(n_recompute_delay*tbs*1000000);
  assert(delta_time > 0);
  double phi, cosPhi=0, sinPhi=0, deltaCosPhi=0, deltaSinPhi=0;
  double phi_end = -2.0*M_PI*(skyfreq + startf + sideband*bwfl*0.5)*
    delTbl[station].calcDelay(time, DelayTable::Fdel);
  double cosPhi_end = cos(phi_end);
  double sinPhi_end = sin(phi_end);

  for (int sample=0; sample<n2fftDC; sample++) {
    if (((int)sample % n_recompute_delay) == 0) {
      phi = phi_end;
      cosPhi = cosPhi_end;
      sinPhi = sinPhi_end;

      time += delta_time;
      phi_end = -2.0*M_PI*(skyfreq + startf + sideband*bwfl*0.5)*
	delTbl[station].calcDelay(time, DelayTable::Fdel);
      cosPhi_end = cos(phi_end);
      sinPhi_end = sin(phi_end);

      deltaCosPhi = (cosPhi_end-cosPhi)/n_recompute_delay;
      deltaSinPhi = (sinPhi_end-sinPhi)/n_recompute_delay;
    }
    
    // 6b)apply normalization and multiply by 2.0
    // NHGK: Why only the real part
    sls[sample].real() *= 2.0;
        
    // 7)subtract dopplers and put real part in Bufs for the current segment
    Bufs[station][n2fftDC*jsegm+sample] = 
      sls[sample].real()*cosPhi - sls[sample].imag()*sinPhi;
    cosPhi += deltaCosPhi;
    sinPhi += deltaSinPhi;

  }
  return true;
}

//Fills Bufs with delay corrected data. This function has to be called
//every time all data in Bufs are processed.
bool DelayCorrection::fill_Bufs()
{
  if (!fill_data_before_delay_correction()) {
    assert(false);
    return false;
  }
  if (!delay_correct()) {
    assert(false);
    return false;
  }

  return true;
}



Log_writer& DelayCorrection::get_log_writer()
{
  return log_writer;
}

//set local delay table parameter
bool DelayCorrection::set_delay_table(int stations, DelayTable &delay_table)
{
  assert(stations >= 0);
  assert((size_t)stations < delTbl.size());
  delTbl[stations]=delay_table;
  
  return true;
}

