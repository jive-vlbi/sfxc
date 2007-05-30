/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */


//sfxc includes
#include "CorrelationCore.h"
#include <assert.h>

CorrelationCore::CorrelationCore()
  :data_writer(NULL)//member initialisation list
{
}

CorrelationCore::CorrelationCore(GenP& GenPrms, int ref_sn)
  :data_writer(NULL)//member initialisation list
{
  set_parameters(GenPrms, ref_sn);
}

void CorrelationCore::set_parameters(GenP& GenPrms, int ref_sn)
{
  nstations = GenPrms.get_nstations();
  if (0 <= ref_sn && ref_sn < nstations) {
    //use a reference station
    nbslns    = 2*nstations-1;
  } else {
    //correlate all baselines
    nbslns    = nstations*(nstations-1)/2 + nstations;
  }
  n2fftcorr = GenPrms.get_n2fft();
  padding   = GenPrms.get_pad();


  segm = new double*[nstations];
  xps = new fftw_complex*[nstations];
  for (int sn=0; sn<nstations; sn++){
    segm[sn] = new double[n2fftcorr*padding];
    xps[sn] = new fftw_complex[n2fftcorr*padding/2+1];
    for (int j=0; j < n2fftcorr*padding; j++) segm[sn][j]=0;
    for (int j=0; j < n2fftcorr*padding/2+1; j++){
      xps[sn][j][0] = 0.0; xps[sn][j][1] = 0.0;
    }
  }
  
  
  norms = new double[nbslns];
  accxps = new fftw_complex*[nbslns];
  for (int j=0; j<nbslns; j++){
    accxps[j] =  new fftw_complex[n2fftcorr*padding/2+1];
  }


  p_r2c = new fftw_plan[nstations];
  //plan the FFTs
  for (int sn = 0; sn < nstations; sn++){
    p_r2c[sn] =
      fftw_plan_dft_r2c_1d(n2fftcorr*padding,segm[sn],xps[sn],FFTW_EXHAUSTIVE);
  }  
 
  ref_station = ref_sn; 
}



CorrelationCore::~CorrelationCore()
{

  delete [] norms;
  for (int j=0; j<nbslns; j++)
    delete [] accxps[j];
  delete [] accxps;
  
  for (int sn=0; sn<nstations; sn++){
    delete [] segm[sn];
    delete [] xps[sn];
  }
  delete [] segm;
  delete [] xps;
  
  for (int sn=0; sn<nstations; sn++)
    fftw_destroy_plan(p_r2c[sn]);
  delete [] p_r2c;
  
}



bool CorrelationCore::init_time_slice()
{
  for (int i = 0; i < nbslns ; i++){
    for (int j = 0 ; j < n2fftcorr*padding/2+1; j++){
      accxps[i][j][0] = 0.0; //real
      accxps[i][j][1] = 0.0; //imaginary
    }
    norms[i] = 0.0;
  }
  return true;
}



void CorrelationCore::correlate_baseline(int station1, int station2, int bsln) {

  for (int j = 0 ; j < n2fftcorr*padding/2 + 1 ; j++){
    //accxps[bsln][j] += xps[station1][j]*conj(xps[station2][j])
    
    accxps[bsln][j][0] = accxps[bsln][j][0] +
    (xps[station1][j][0] * xps[station2][j][0]) +
    (xps[station1][j][1] * xps[station2][j][1]);
    
    accxps[bsln][j][1] = accxps[bsln][j][1] +
    (xps[station1][j][1] * xps[station2][j][0]) -
    (xps[station1][j][0] * xps[station2][j][1]);
    
  }
}



bool CorrelationCore::correlate_segment(double** in_segm)
{
  int bsln = 0; //initialise basline number    
  
  // FWD FFT each station + calculate the auto products
  for (int sn = 0 ; sn < nstations; sn++){
    //fill the local segment with data from the relevant station
    for (int i=0; i< n2fftcorr; i++) segm[sn][i]=in_segm[sn][i];
    //execute FFT real to complex. input: segm -> result: xps
    fftw_execute(p_r2c[sn]);
    for (int j = 0 ; j < n2fftcorr*padding/2 + 1 ; j++){
      //accxps[bsln][j] += xps[sn][j]*conj(xps[sn][j]);
      accxps[bsln][j][0] = accxps[bsln][j][0] +
      (xps[sn][j][0] * xps[sn][j][0]) + (xps[sn][j][1] * xps[sn][j][1]);
      //accxps[bsln][j][1] imaginary part stays zero
    }
    bsln++;
  }

  if (0 <= ref_station && ref_station < nstations) {
    //calculate the correlations using only the reference station
    for (int sno = 0; sno < nstations ; sno ++){
      if(sno != ref_station) {
        correlate_baseline(ref_station, sno, bsln);
        bsln++;
      }
    }
  } else {
    //calculate the correlations for all base lines
    for (int sn = 0 ; sn < nstations - 1; sn++){
      for (int sno = sn + 1; sno < nstations ; sno ++){
        correlate_baseline(sn, sno, bsln);
        bsln++;
      }
    }
  }

  return true;
}




void CorrelationCore::normalise_correlation(int station1, int station2, int bsln)
{
  norms[bsln] = sqrt(norms[station1]*norms[station2]);
  for (int j = 0 ; j < n2fftcorr*padding/2 + 1 ; j++){
    accxps[bsln][j][0] = accxps[bsln][j][0] / norms[bsln];
    accxps[bsln][j][1] = accxps[bsln][j][1] / norms[bsln];
  }
}



bool CorrelationCore::average_time_slice()
{

  int bsln = 0;//initialise baseline counter
  //auto product normalisation, mean pwr = 1
  for (int sn = 0 ; sn < nstations ; sn++){
    for (int j = 0; j < n2fftcorr*padding/2 + 1; j++){
      norms[bsln] = norms[bsln] + accxps[bsln][j][0];
    }
    norms[bsln] = norms[bsln] / (double)(n2fftcorr*padding/2 + 1);
    for (int j = 0; j < n2fftcorr*padding/2 + 1; j++){
      accxps[bsln][j][0] = accxps[bsln][j][0] / norms[bsln];
    }
    bsln++;
  }


  if (0 <= ref_station && ref_station < nstations) {
    //cross product normalisation w.r.t reference station
    for (int sno = 0; sno < nstations ; sno ++){
      if (sno != ref_station) {
        normalise_correlation(ref_station,sno,bsln);
        bsln++;
      }
    }
  } else {
    //cross product normalisation for all possible base lines
    for (int sn = 0 ; sn < nstations - 1; sn++){
      for (int sno = sn + 1; sno < nstations ; sno ++){
        normalise_correlation(sn,sno,bsln);
        bsln++;
      }
    }
  }

  return true;
}



bool CorrelationCore::write_time_slice()
{
  //write normalized correlation results to output file
  //NGHK: Make arrays consecutive to be able to write all data at once
  for (int bsln = 0; bsln < nbslns; bsln++){
    UINT64 nWrite = sizeof(fftw_complex)*(n2fftcorr*padding/2+1); 
    UINT64 written = get_data_writer().
      put_bytes(nWrite, (char *)(accxps[bsln]));
    if (nWrite != written) return false;
  }

  return true;
}


Data_writer& CorrelationCore::get_data_writer()
{
  assert(data_writer != NULL);
  return *data_writer;
}


void CorrelationCore::set_data_writer(Data_writer *data_writer_)
{
  data_writer=data_writer_;
}
