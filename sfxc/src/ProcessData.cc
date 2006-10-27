/*
Functions for processing the recorded data
- extract data from file
- optional filtering (not implemented yet)
- delay correction
- auto and cross- correlation

Author     : RHJ Oerlemans
StartDate  : 20061005
Last change: 20061005

*/

//these defines have to be the first in source file
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

//enable define on 32 bit CPU, disable on 64 bit CPU
#define THIRTYTWO

//32 bit machine define,
//use open, lseek, off_t in stead off open64, lseek64, off64_t
#ifdef THIRTYTWO
#define _FILE_OFFSET_BITS 64
#endif


//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <complex.h> //depricated
#include <fftw3.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <string>
#include <complex>
using namespace std;

//includes for system calls
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "gen_defines.h"

//constants
#include "constVars.h"


//class and function definitions
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "InData.h"
#include "ProcessData.h"

//global variables
extern RunP  RunPrms;
extern GenP  GenPrms;


//***************************************************************************
// fill the buffers with pre-correlation data and correlate
//***************************************************************************
int CorrelateBufs(StaP StaPrms[], INT64 sliceStartByte[][NcoresMax],
  INT64 sliceStopByte[][NcoresMax], int core)
{

  //declarations
  int i,j;
  int nstations, sn; //nr of stations and station counter
  int n2fft; //FFT length in correlation
  int pad; //padding with zeros for invecs
  INT64 Nsamp2Avg; //nr of samples to average the correlation
  INT64 Nsegm2Avg; //nr of fourier segments to average the correlation
  INT64 BytePtr; //location of input file read pointer
  INT64 BufPtr; //location of the buffer pointer 
  int nbslns; //number of baselines
  float *norms; //normalization coeffs
  double **Bufs; //buffers with pre-correlated data ready for correlation
  double **invecs;//input vectors for FFT operation
  fftw_complex **xps; //xps: result vectors from FFT
  fftw_complex **accxps; //accumulated cross and auto powers Real and Imaginary part
  fftw_plan *fwd_plans; //FFT plans
  

  //initialisations and allocations  
  nstations = GenPrms.get_nstations();
  nbslns = nstations*(nstations-1)/2;
  n2fft = GenPrms.get_n2fft();
  pad = GenPrms.get_pad();
  Nsamp2Avg = GenPrms.get_nsamp2avg();
  Nsegm2Avg = Nsamp2Avg/n2fft;
    
  norms = new float[nbslns];

  Bufs =new double*[nstations];
  for (sn=0; sn<nstations; sn++){
    Bufs[sn] = new double[BufSize];
  }
  
  invecs = new double*[nstations];
  for (sn=0; sn<nstations; sn++){
    invecs[sn] = new double[n2fft*pad];
    for (j=0; j < n2fft*pad; j++){
      invecs[sn][j] = 0.0;
    }
  }
  
  xps = new fftw_complex*[nstations];
  for (sn=0; sn<nstations; sn++){
    xps[sn] = new fftw_complex[n2fft*pad/2+1];
  }

  accxps = new fftw_complex*[nbslns];
  for (j=0; j<nbslns; j++){
    accxps[j] =  new fftw_complex[n2fft*pad/2+1];
  }

  fwd_plans = new fftw_plan[nstations];  
  
  //plan the FFTs
  if (RunPrms.get_messagelvl()> 0)
    cout << "\nPlanning the forward FFTs!\n";
  for (sn = 0; sn < nstations; sn++){
    fwd_plans[sn] =
      fftw_plan_dft_r2c_1d(n2fft*pad,invecs[sn],xps[sn],FFTW_EXHAUSTIVE);
  }

  //open the output file

  //loop initializations
  BytePtr = sliceStartByte[0][core];
  BufPtr = BufSize;
  
  //while loop for processing from startbyte until stopbyte
//  while (BytePtr < sliceStopByte[0][core]) {
  while (1) {
  
    //zero accxps array and norms array
    for (i = 0; i < nbslns ; i++){
      for (j = 0 ; j < n2fft*pad/2+1; j++){
        accxps[i][j][0] = 0.0; //real 
        accxps[i][j][1] = 0.0; //imaginary
      }
      norms[i] = 0.0;
    }

    //process all the segments in the frame
    for (j = 0 ; j < Nsegm2Avg ; j++){

      //read data from data files, do pre-correlation, 
      //and put results in Bufs. 
      if ( (BufPtr+n2fft)>BufSize ) {
        fill_Bufs(StaPrms,sliceStartByte,sliceStopByte,BytePtr,Bufs);
        BufPtr=0;
      }
      //get data from Bufs and put in invecs
     // fetch_invecs(&BufPtr, nfiles, N2FFT, invecs, BUFS, DNSH);
     // bsln = 0;
      
    }


    break; //for test purpose while loop only once
  } // End while loop for processing from startbyte until stopbyte
  
  //close output file
  
  //free allocated memory   
  delete [] norms;
    
  for (sn=0; sn<nstations; sn++)
    delete [] Bufs[sn];
  delete [] Bufs;
  
  for (sn=0; sn<nstations; sn++)
    delete [] invecs[sn];
  delete [] invecs;
  
  for (sn=0; sn<nstations; sn++)
    delete [] xps[sn];
  delete [] xps;
  
  for (j=0; j<nbslns; j++)
    delete [] accxps[j];
  delete [] accxps;
  
  //destroy fftw plans
  for (sn=0; sn<nstations; sn++)
    fftw_destroy_plan(fwd_plans[sn]);

  return 1;

}



//***************************************************************************
//read data from data files do pre-correlation, and put results in Bufs.
//***************************************************************************
int fill_Bufs(StaP StaPrms[], INT64 sliceStartByte[][NcoresMax],
  INT64 sliceStopByte[][NcoresMax], INT64 BytePtr, double **Bufs)
{
  int retval = 0;
  int nstations, sn; //nr of stations and station counter
  int lsegm; //fourier length of a segment in the pre-correlation
  INT64 Nsegm2PrC; //nr of segments to precorrelate, result in Bufs
  INT64 segm; //segment counter
  double **Mk4Frame; //Mk4 data frames for all stations in memory
  INT64 *FL, *FC;//
  
  //initialisations and allocations  
  nstations = GenPrms.get_nstations();
  lsegm = GenPrms.get_lsegm();
  cout << "lsegm:" << lsegm << endl;
  Nsegm2PrC = BufSize/lsegm;
  cout << "Nsegm2PrC:" << Nsegm2PrC << endl;

  FL = new INT64[nstations];
  FC = new INT64[nstations];
  Mk4Frame = new double*[nstations];
  for (sn=0; sn<nstations; sn++) {
    FL[sn] = FC[sn] = frameMk4*StaPrms[sn].get_fo();
    Mk4Frame[sn] = new double[frameMk4*StaPrms[sn].get_fo()];
  }
  
  for (sn=0; sn<nstations; sn++){
    for (segm=0; segm<Nsegm2PrC; segm++) {
      //if data in frame buffer is processed
      //get new data from Mk4 file in frame buffer
      if ( FL[sn] == FC[sn] ) {
        cout << "read Mk4 data file" << endl;
        FC[sn] = 0;
      }
      //get data from Mk4Frame into segment
      
    }
  }  
  

  //free allocated memory
  for (sn=0; sn<nstations; sn++)
    delete [] Mk4Frame[sn];
  delete [] Mk4Frame;

  
  return retval;
}
