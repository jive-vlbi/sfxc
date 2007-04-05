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

CorrelationCore::CorrelationCore(GenP& GenPrms)
  :data_writer(NULL)//member initialisation list
{
  set_parameters(GenPrms);
}

void CorrelationCore::set_parameters(GenP& GenPrms)
{
  nstations = GenPrms.get_nstations();
  nbslns    = nstations*(nstations-1)/2 + nstations;
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



void CorrelationCore::init_time_slice()
{
  for (int i = 0; i < nbslns ; i++){
    for (int j = 0 ; j < n2fftcorr*padding/2+1; j++){
      accxps[i][j][0] = 0.0; //real
      accxps[i][j][1] = 0.0; //imaginary
    }
    norms[i] = 0.0;
  }
}



void CorrelationCore::correlate_segment(double** in_segm)
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
          
    // calculate the cross products
    for (int sn = 0 ; sn < nstations - 1; sn++){
      for (int sno = sn + 1; sno < nstations ; sno ++){
        for (int j = 0 ; j < n2fftcorr*padding/2 + 1 ; j++){
          //accxps[bsln][j] += xps[sn][j]*conj(xps[sno][j])
          
          accxps[bsln][j][0] = accxps[bsln][j][0] +
          (xps[sn][j][0] * xps[sno][j][0]) +
          (xps[sn][j][1] * xps[sno][j][1]);
          
          accxps[bsln][j][1] = accxps[bsln][j][1] +
          (xps[sn][j][1] * xps[sno][j][0]) -
          (xps[sn][j][0] * xps[sno][j][1]);
          
        }
        bsln++;
      }
    }
    
}



void CorrelationCore::average_time_slice()
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
  //cross product normalisation
  for (int sn = 0 ; sn < nstations - 1; sn++){
    for (int sno = sn + 1; sno < nstations ; sno ++){
      norms[bsln] = sqrt(norms[sn]*norms[sno]);
      for (int j = 0 ; j < n2fftcorr*padding/2 + 1 ; j++){
        accxps[bsln][j][0] = accxps[bsln][j][0] / norms[bsln];
        accxps[bsln][j][1] = accxps[bsln][j][1] / norms[bsln];
      }
      bsln++;
    }
  }


}



void CorrelationCore::write_time_slice()
{
  //TODO RHJO: test
  //write normalized correlation results to output file
  //NGHK: Make arrays consecutive to be able to write all data at once
  for (int bsln = 0; bsln < nbslns; bsln++){
    UINT64 nWrite = sizeof(fftw_complex)*(n2fftcorr*padding/2+1); 
    UINT64 written = get_data_writer().
      put_bytes(nWrite, (char *)(accxps[bsln]));
    assert(nWrite == written);
    
  }


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
