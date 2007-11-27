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
#include "Output_header.h"
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

void CorrelationCore::set_parameters(Correlation_parameters &corr_param_)
{
  corr_param = corr_param_;
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

  segm = new float*[nstations]; 
  xps = new fftwf_complex*[nstations];
  for (int sn=0; sn<nstations; sn++){
    segm[sn] = new float[n2fftcorr*padding];
    xps[sn] = new fftwf_complex[n2fftcorr*padding/2+1];
    for (int j=0; j < n2fftcorr*padding; j++) segm[sn][j]=0;
    for (int j=0; j < n2fftcorr*padding/2+1; j++){
      xps[sn][j][0] = 0.0; xps[sn][j][1] = 0.0;
    }
  }
  
  
  norms = new float[nbslns];
  accxps = new fftwf_complex*[nbslns];
  for (int j=0; j<nbslns; j++){
    accxps[j] =  new fftwf_complex[n2fftcorr*padding/2+1];
  }


  p_r2c = new fftwf_plan[nstations];
  //plan the FFTs
  for (int sn = 0; sn < nstations; sn++){
    p_r2c[sn] =
      fftwf_plan_dft_r2c_1d(n2fftcorr*padding,segm[sn],xps[sn],FFTW_EXHAUSTIVE);
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
      fftwf_destroy_plan(p_r2c[sn]);
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



bool CorrelationCore::correlate_segment(float** in_segm)
{
  assert(parameters_set);
  int bsln = 0; //initialise basline number    
  
  // FWD FFT each station + calculate the auto products
  for (int sn = 0 ; sn < nstations; sn++){
    //fill the local segment with data from the relevant station
    for (int i=0; i< n2fftcorr; i++) segm[sn][i]=in_segm[sn][i];
    //execute FFT real to complex. input: segm -> result: xps
    assert(segm[sn][0] == segm[sn][0]); // Not NaN
    fftwf_execute(p_r2c[sn]);
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
    norms[bsln] = norms[bsln] / (float)(n2fftcorr*padding/2 + 1);
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
  // write a header per channel and
  // write a header per baseline

  DEBUG_MSG("Output_header_timeslice: " << sizeof(Output_header_timeslice));
  DEBUG_MSG("Output_header_baseline:  " << sizeof(Output_header_baseline));
  
  int nr_corr = (corr_param.stop_time-corr_param.start_time)/corr_param.integration_time;

  int bsln = 0;//initialise baseline counter
  int station1_bsln[nbslns];
  int station2_bsln[nbslns];
  int pol1[nbslns];
  int pol2[nbslns];
  int corr_param_polarisation;
  if(corr_param.polarisation == 'R'){
    corr_param_polarisation =0;
  } else if (corr_param.polarisation == 'L'){
    corr_param_polarisation =1;
  }

  //auto product normalisation, mean pwr = 1
  for (int sn = 0 ; sn < nstations ; sn++){
    station1_bsln[bsln] = sn;
    station2_bsln[bsln] = sn;
    pol1[bsln] = corr_param_polarisation;
    pol2[bsln] = corr_param_polarisation;
    DEBUG_MSG("========= 1. auto coorelation ========");
    DEBUG_MSG("baseline --> : " << bsln);
    DEBUG_MSG("station1 --> : " << sn);
    DEBUG_MSG("station2 --> : " << sn);
    DEBUG_MSG("polarization1 --> : " << pol1[bsln]);
    DEBUG_MSG("polarization2 --> : " << pol2[bsln]);
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
          if(sn>=nstations_2){
            pol1[bsln] = 1-corr_param_polarisation;  
          } else {
            pol1[bsln] = corr_param_polarisation;
          }
          pol2[bsln] = corr_param_polarisation;
          station1_bsln[bsln] = sn;
          station2_bsln[bsln] = reference_station;
          DEBUG_MSG("========= 2. cross with ref ========");
          DEBUG_MSG("baseline --> : " << bsln);
          DEBUG_MSG("station1 --> : " << sn);
          DEBUG_MSG("station2 --> : " << reference_station);        
          DEBUG_MSG("polarization1 --> : " << pol1[bsln]);
          DEBUG_MSG("polarization2 --> : " << pol2[bsln]);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
      for (int sn = 0 ; sn < nstations; sn++){
        if ((sn != reference_station) && 
            (sn != reference_station+nstations_2)) {
          // Do not cross correlate the 
          // two polarisations of the same station
          if(sn>=nstations_2){
            pol1[bsln] = 1-corr_param_polarisation;  
          } else {
            pol1[bsln] = corr_param_polarisation;
          }
          pol2[bsln] = 1-corr_param_polarisation;
          station1_bsln[bsln] = sn;
          station2_bsln[bsln] = reference_station;    
          DEBUG_MSG("========= 2. cross with ref ========");
          DEBUG_MSG("baseline --> : " << bsln);
          DEBUG_MSG("station1 --> : " << sn);
          DEBUG_MSG("station2 --> : " << reference_station);
          DEBUG_MSG("polarization1 --> : " << pol1[bsln]);
          DEBUG_MSG("polarization2 --> : " << pol2[bsln]);
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
            if(sn>=nstations_2){
              pol1[bsln] = 1-corr_param_polarisation;  
            } else {
              pol1[bsln] = corr_param_polarisation;
            }
            if(sno>=nstations_2){
              pol2[bsln] = 1-corr_param_polarisation;
            } else {
              pol2[bsln] = corr_param_polarisation;
            }
            station1_bsln[bsln] = sn;
            station2_bsln[bsln] = sno;
            DEBUG_MSG("========= 3. cross no ref ========");
            DEBUG_MSG("baseline --> : " << bsln);
            DEBUG_MSG("station1 --> : " << sn);
            DEBUG_MSG("station2 --> : " << sno);
            DEBUG_MSG("polarization1 --> : " << pol1[bsln]);
            DEBUG_MSG("polarization2 --> : " << pol2[bsln]);
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
          pol1[bsln] = corr_param_polarisation;
          pol2[bsln] = corr_param_polarisation;
          station1_bsln[bsln] = sn;
          station2_bsln[bsln] = reference_station;
          DEBUG_MSG("========= 4. no cross with ref ========");
          DEBUG_MSG("baseline --> : " << bsln);
          DEBUG_MSG("station1 --> : " << sn);
          DEBUG_MSG("station2 --> : " << reference_station);
          DEBUG_MSG("polarization1 --> : " << pol1[bsln]);
          DEBUG_MSG("polarization2 --> : " << pol2[bsln]);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
    } else {
      // no cross polarisation without a reference station

      for (int sn = 0 ; sn < nstations - 1; sn++){
        for (int sno = sn + 1; sno < nstations ; sno ++){
          pol1[bsln] = corr_param_polarisation;
          pol2[bsln] = corr_param_polarisation;
          station1_bsln[bsln] = sn;
          station2_bsln[bsln] = sno;
          DEBUG_MSG("========= 5. no cross no ref ========");
          DEBUG_MSG("baseline --> : " << bsln);
          DEBUG_MSG("polarization1 --> : " << pol1[bsln]);
          DEBUG_MSG("polarization2 --> : " << pol2[bsln]);
          DEBUG_MSG("station1 --> : " << sn);
          DEBUG_MSG("station2 --> : " << sno);
          bsln++;
          assert(bsln <= nbslns);
        }
      }
    }
  }
  assert(bsln == nbslns);
  
  Output_header_timeslice htimeslice;
  Output_header_baseline hbaseline;
  
  htimeslice.number_baselines = bsln;
  htimeslice.integration_slice = (corr_param.stop_time - corr_param.start_time)
                                  / corr_param.integration_time;
  htimeslice.number_uvw_coordinates = 3;
 
  DEBUG_MSG("TIMESLICE HEADER:number of baselines --> : " << htimeslice.number_baselines);
  DEBUG_MSG("TIMESLICE HEADER:integration slice --> : " << htimeslice.integration_slice);
  DEBUG_MSG("TIMESLICE HEADER:number of uvw coord. --> : " << htimeslice.number_uvw_coordinates);

  //write normalized correlation results to output file
  //NGHK: Make arrays consecutive to be able to write all data at once
  
  uint64_t nWrite = sizeof(htimeslice);
  uint64_t written = get_data_writer().put_bytes(nWrite, (char *)&htimeslice);

  for (int bsln = 0; bsln < nbslns; bsln++){
    hbaseline.weight = 0;       // The number of good samples
    hbaseline.station_nr1 = (uint8_t)station1_bsln[bsln];  // Station number in the vex-file
    hbaseline.station_nr2 = (uint8_t)station2_bsln[bsln];  // Station number in the vex-file
    hbaseline.polarisation1 = (unsigned char)pol1[bsln]; // Polarisation for the first station
                            // (RCP: 0, LCP: 1)
    hbaseline.polarisation2 = (unsigned char)pol2[bsln]; // Polarisation for the second station
    if(corr_param.sideband=='U'){
      hbaseline.sideband = 0;      // Upper or lower sideband (LSB: 0, USB: 1)
    }else if(corr_param.sideband=='L'){
      hbaseline.sideband = 1;
    }
    hbaseline.frequency_nr = (unsigned char)corr_param.channel_nr;       // The number of the channel in the vex-file,
                            // sorted increasingly
      // 1 byte left:
    hbaseline.empty = ' ';
    DEBUG_MSG("BASELINE HEADER:weight --> : " << (int)hbaseline.weight);
    DEBUG_MSG("BASELINE HEADER:station1 --> : " << (int)hbaseline.station_nr1);
    DEBUG_MSG("BASELINE HEADER:station2 --> : " << (int)hbaseline.station_nr2);
    DEBUG_MSG("BASELINE HEADER:polaris1 --> : " << (int)hbaseline.polarisation1);
    DEBUG_MSG("BASELINE HEADER:polaris2 --> : " << (int)hbaseline.polarisation2);
    DEBUG_MSG("BASELINE HEADER:sideband --> : " << (int)hbaseline.sideband);
    DEBUG_MSG("BASELINE HEADER:freq_nr --> : " << (int)hbaseline.frequency_nr);
    DEBUG_MSG("BASELINE HEADER:empty --> : " << (char)hbaseline.empty);
    
    nWrite = sizeof(hbaseline);
    written = get_data_writer().put_bytes(nWrite, (char *)&hbaseline);
    nWrite = sizeof(fftwf_complex)*(n2fftcorr*padding/2+1);
    written = get_data_writer().
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


void CorrelationCore::set_data_writer(boost::shared_ptr<Data_writer> data_writer_)
{
  data_writer=data_writer_;
}
