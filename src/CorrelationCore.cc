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
#include <utils.h>

CorrelationCore::CorrelationCore(Log_writer &lw) 
  : accxps(NULL),
    segm(NULL),
    xps(NULL),
    norms(NULL),
    p_r2c(NULL), 
    log_writer(lw),
    parameters_set(false)
{
}

CorrelationCore::CorrelationCore(Log_writer &lw, 
                                 Correlation_parameters &corr_param)
  : accxps(NULL),
    segm(NULL),
    xps(NULL),
    norms(NULL),
    p_r2c(NULL),
    log_writer(lw),
    parameters_set(false)
{
  set_parameters(corr_param);
}

void CorrelationCore::set_parameters(Correlation_parameters &corr_param)
{
  parameters_set = true;

  cross_polarize = corr_param.cross_polarize;
  reference_station = corr_param.reference_station;

  nstations = corr_param.station_streams.size();

  //int nbslns;
  if (cross_polarize) {
    int nAutos = nstations/2;
    int nCrosses = nstations/2*(nstations/2-1)/2;
    if (reference_station >= 0) {
      nbslns = 2*nAutos + 4*(nAutos-1);
    } else {
      nbslns = 2*nAutos + 4*nCrosses;
    }
  } else {
    int nAutos = nstations;
    int nCrosses = nstations*(nstations-1)/2;
    if (reference_station >= 0) {
      nbslns = 2*nAutos - 1;
    } else {
      nbslns = nAutos + nCrosses;
    }
  }

  n2fftcorr = corr_param.number_channels;
  padding   = PADDING;


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
  if (!parameters_set) {
    return;
  }
  if (norms != NULL) {
    delete [] norms;
  }
  if (accxps != NULL) {
    for (int j=0; j<nbslns; j++)
      delete [] accxps[j];
    delete [] accxps;
  }
  if (segm != NULL) {
    for (int sn=0; sn<nstations; sn++){
      delete [] segm[sn];
    }
    delete [] segm;
  }
  if (xps != NULL) {
    for (int sn=0; sn<nstations; sn++){
      delete [] xps[sn];
    }
    delete [] xps;
  }
  if (p_r2c != NULL) {
    for (int sn=0; sn<nstations; sn++)
      fftw_destroy_plan(p_r2c[sn]);
    delete [] p_r2c;
  }
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
    accxps[bsln][j][0] +=
    (xps[station1][j][0] * xps[station2][j][0]) +
    (xps[station1][j][1] * xps[station2][j][1]);
    
    accxps[bsln][j][1] +=
    (xps[station1][j][1] * xps[station2][j][0]) -
    (xps[station1][j][0] * xps[station2][j][1]);
  }
}



bool CorrelationCore::correlate_segment(double** in_segm)
{
  assert(parameters_set);
  int bsln = 0; //initialise basline number    
  
  // FWD FFT each station + calculate the auto products
  for (int sn = 0 ; sn < nstations; sn++){
    //fill the local segment with data from the relevant station
    for (int i=0; i< n2fftcorr; i++) segm[sn][i]=in_segm[sn][i];
    //execute FFT real to complex. input: segm -> result: xps
    assert(segm[sn][0] == segm[sn][0]); // Not NaN
    fftw_execute(p_r2c[sn]);
    for (int j = 0 ; j < n2fftcorr*padding/2 + 1 ; j++){
      assert(xps[sn][j][0] == xps[sn][j][0]); // Not NaN
      //accxps[bsln][j] += xps[sn][j]*conj(xps[sn][j]);
      accxps[bsln][j][0] += 
        (xps[sn][j][0] * xps[sn][j][0]) + (xps[sn][j][1] * xps[sn][j][1]);
      //accxps[bsln][j][1] imaginary part stays zero
    }

    bsln++;
  }

  if (cross_polarize) {
    int nstations_2 = nstations/2;
    if (reference_station >= 0) {
      // cross polarize with a reference station

      for (int sn = 0 ; sn < nstations; sn++){
        if ((sn != reference_station) && 
            (sn != (reference_station+nstations_2))) {
          correlate_baseline(sn, reference_station, bsln);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
      for (int sn = 0 ; sn < nstations; sn++){
        if ((sn != reference_station) && 
            (sn != (reference_station+nstations_2))) {
          correlate_baseline(sn, reference_station+nstations_2, bsln);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
    } else {
      // cross polarize without a reference station
      for (int sn = 0 ; sn < nstations - 1; sn++){
        for (int sno = sn + 1; sno < nstations ; sno ++){
          if (sn+nstations_2 != sno) {
            // Do not cross correlate the 
            // two polarisations of the same station
            correlate_baseline(sn, sno, bsln);
            bsln++;
            assert(bsln <= nbslns);
          }
        }
      }
    }
  } else {
    if (reference_station >= 0) {
      // no cross polarisation with a reference station

      for (int sn = 0 ; sn < nstations; sn++){
        if (sn != reference_station) {
          correlate_baseline(sn, reference_station, bsln);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
    } else {
      // no cross polarisation without a reference station

      for (int sn = 0 ; sn < nstations - 1; sn++){
        for (int sno = sn + 1; sno < nstations ; sno ++){
          correlate_baseline(sn, sno, bsln);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
    }
  }
  assert(bsln == nbslns);

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
    for (int j = 0; j < n2fftcorr*padding/2 + 1; j++) {
      norms[bsln] += accxps[bsln][j][0];
    }
    norms[bsln] = norms[bsln] / (double)(n2fftcorr*padding/2 + 1);
    for (int j = 0; j < n2fftcorr*padding/2 + 1; j++){
      accxps[bsln][j][0] /= norms[bsln];
    }
    bsln++;
  }


  //cross product normalisation for all possible base lines
  if (cross_polarize) {
    int nstations_2 = nstations/2;
    if (reference_station >= 0) {
      // cross polarize with a reference station
      int nstations_2 = nstations/2;
      for (int sn = 0 ; sn < nstations; sn++){
        if ((sn != reference_station) && 
            (sn != reference_station+nstations_2)) {
          // Do not cross correlate the 
          // two polarisations of the same station
          normalise_correlation(sn,reference_station,bsln);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
      for (int sn = 0 ; sn < nstations; sn++){
        if ((sn != reference_station) && 
            (sn != reference_station+nstations_2)) {
          // Do not cross correlate the 
          // two polarisations of the same station
          normalise_correlation(sn,reference_station+nstations_2,bsln);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
    } else {
      // cross polarize without a reference station
      int nstations_2 = nstations/2;
      for (int sn = 0 ; sn < nstations - 1; sn++){
        for (int sno = sn + 1; sno < nstations ; sno ++){
          if (sn + nstations_2 != sno) {
            // Do not cross correlate the 
            // two polarisations of the same station
            normalise_correlation(sn,sno,bsln);
            bsln++;
            assert(bsln <= nbslns);
          }
        }
      }
    }
  } else {
    if (reference_station >= 0) {
      // no cross polarisation with a reference station
      for (int sn = 0 ; sn < nstations; sn++){
        if (sn != reference_station) {
          // Do not cross correlate the 
          // two polarisations of the same station
          normalise_correlation(sn,reference_station,bsln);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
    } else {
      // no cross polarisation without a reference station

      for (int sn = 0 ; sn < nstations - 1; sn++){
        for (int sno = sn + 1; sno < nstations ; sno ++){
          normalise_correlation(sn,sno,bsln);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
    }
  }
  assert(bsln == nbslns);

  return true;
}



bool CorrelationCore::write_time_slice()
{
  //write normalized correlation results to output file
  //NGHK: Make arrays consecutive to be able to write all data at once
  uint64_t nWrite = sizeof(fftw_complex)*(n2fftcorr*padding/2+1);
  for (int bsln = 0; bsln < nbslns; bsln++){
    uint64_t written = get_data_writer().
      put_bytes(nWrite, (char *)(accxps[bsln]));
    if (nWrite != written) return false;
  }
  DEBUG_MSG("Size of time slice: " << nWrite*nbslns);
  return true;
}


Data_writer& CorrelationCore::get_data_writer()
{
  assert(data_writer != NULL);
  return *data_writer;
}


void CorrelationCore::set_data_writer(boost::shared_ptr<Data_writer> data_writer_)
{
  data_writer=data_writer_;
}
