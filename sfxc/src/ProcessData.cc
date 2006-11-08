/*
Functions for processing the recorded data
- extract data from file
- optional filtering (not implemented yet)
- delay correction
- auto and cross- correlation

Author     : RHJ Oerlemans
StartDate  : 20061005
Last change: 20061027

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
#include "constPrms.h"


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
extern StaP  StaPrms[NstationsMax];
extern double PI;
extern INT64 sliceStartByte[NstationsMax][NcoresMax];
extern INT64 sliceStopByte [NstationsMax][NcoresMax];
extern INT64 sliceStartTime [NcoresMax];
extern INT64 sliceStopTime  [NcoresMax];
extern INT64 sliceTime;


//***************************************************************************
//prototypes for local functions
//***************************************************************************
int fill_Bufs(int *inFile,
  double **Bufs, double **dcBufPrev, int BufSize,
  double **Mk4frame, INT64 *FL, INT64 *FC,
  double *signST, double *magnST, INT64 *Nsamp,
  fftw_complex *sls, fftw_complex *spls, fftw_plan& planFW, fftw_plan& planBW,
  double tbs, double *fs, int Nf, double& timePtr,
  INT64 *delaydt, double **cdel, double **fdel, INT64 ndel, int core);

int fetch_invecs(INT64& BufPtr, int nstations, int n2fft,
  double **invecs, double **Bufs);

  
//***************************************************************************
// fill the buffers with pre-correlation data and correlate
//***************************************************************************
int CorrelateBufs(int core)
{
  //declarations
  int i,j,l;
  int nstations, sn, sno; //nr of stations and station counters
  int n2fft; //FFT length in correlation
  int pad; //padding with zeros for invecs
  int *inFile; //file descriptor input file
  INT64 Nsamp2Avg; //nr of samples to average the correlation
  INT64 Nsegm2Avg; //nr of fourier segments to average the correlation
  INT64 segm; //segment number in for loop
  INT64 TenPct;
  INT64 *BytePtr; //location of input file read pointer
  INT64 BufPtr; //location of the buffer pointer
  INT64 statusPtr; // status of file pointer
  INT64 *Nsamp;
  double timePtr; //a time pointer in micro seconds
  int nbslns; //number of baselines, cross and auto correlations
  int bsln; // baseline number
  float *norms; //normalization coeffs
  double **Bufs; //buffers with pre-correlated data ready for correlation
  double **dcBufPrev; //buffers with data for delay correction
  int BufSize; // size of one buffer in Bufs
  double **invecs;//input vectors for FFT operation
  fftw_complex **xps; //xps: result vectors from FFT
  fftw_complex **accxps; //accumulated cross and auto powers Real and Imaginary part
  fftw_plan *fwd_plans; //FFT plans
  double **Mk4frame; //channel data for one frame
  INT64 *FL; //Mk4 frame length
  INT64 *FC; //Mk4 frame counter
  double *signST, *magnST;//sign and magnitude statistics
  INT64 loop; //while loop counter
  double SR, tbs; //sample rate and time between samples
  double dfr, *fs; // delta frequency in frequency scale, frequency scale
  int    jf, Nf; //nr of frequencies in frequency scale
  double **cdel, **mdel, **rdel, **fdel; //arrays for delay tables
  INT64 *delaydt, **tdel, ndel; // time in delay table in usec and number of delay lines
  FILE *outP; //output result file


  //declarations for fftw in delay correction
  fftw_complex *sls, *spls; //FW:in,out; BW: out,in
  fftw_plan    planFW, planBW;//plans for forward and backward fft 
  int lsegm; //fourier length of a segment in the pre-correlation

  //initialisations and allocations  
  nstations = GenPrms.get_nstations();
  nbslns    = nstations*(nstations-1)/2 + nstations; //cross + auto 
  n2fft     = GenPrms.get_n2fft();
  lsegm     = GenPrms.get_lsegm();
  pad       = GenPrms.get_pad();
  Nsamp2Avg = GenPrms.get_nsamp2avg();
  Nsegm2Avg = Nsamp2Avg/n2fft;
  TenPct    = Nsegm2Avg/10;
  SR=2.0*GenPrms.get_bwfl()*GenPrms.get_ovrfl();//sample rate
  tbs=1.0/SR; //time between samples in seconds
  BufSize   = BufTime * (SR/1000000.0);
  Nf = lsegm/2+1;
  dfr = 1.0/(lsegm*tbs);
  fs = new double[Nf];
  for (jf=0; jf<Nf; jf++) //frequency scale in the segment
    fs[jf]=jf*dfr-0.5*GenPrms.get_bwfl()-GenPrms.get_foffset();
  
  if (RunPrms.get_messagelvl()> 1){
    cout << "BufSize=" << BufSize << endl;
    cout << "Nsamp2Avg=" << Nsamp2Avg << endl;
    cout << "n2fft    =" << n2fft << endl;
    cout << "Nsegm2Avg=" << Nsegm2Avg << endl;
    cout << "SR  = " << SR << endl;
    cout << "tbs = " << tbs << endl;
    cout << "Nf  = " << Nf << endl;
    cout << "dfr = " << dfr << endl;
  }
    
  norms = new float[nbslns];
  inFile = new int[nstations];
  BytePtr = new INT64[nstations];
  signST = new double[nstations];
  magnST = new double[nstations];
  Nsamp = new INT64[nstations];
  
  Bufs =new double*[nstations];
  dcBufPrev =new double*[nstations];
  for (sn=0; sn<nstations; sn++){
    Bufs[sn] = new double[BufSize];
    dcBufPrev[sn] = new double[2*BufSize];
  }
  
  invecs = new double*[nstations];
  for (sn=0; sn<nstations; sn++){
    invecs[sn] = new double[n2fft*pad];
    for (j=0; j < n2fft*pad; j++){
      invecs[sn][j] = 0.0;
    }
  }
  
  Mk4frame = new double *[nstations];
  FL = new INT64 [nstations];
  FC = new INT64 [nstations];
  for (sn=0; sn<nstations; sn++){
    FL[sn] = frameMk4*StaPrms[sn].get_fo();
    FC[sn] = FL[sn];
    Mk4frame[sn] = new double[FL[sn]];
  }
  
  xps = new fftw_complex*[nstations];
  for (sn=0; sn<nstations; sn++){
    xps[sn] = new fftw_complex[n2fft*pad/2+1];
    for (j=0; j < n2fft*pad/2+1; j++){
      xps[sn][j][0] = 0.0;
      xps[sn][j][1] = 0.0;
    }
  }

  accxps = new fftw_complex*[nbslns];
  for (j=0; j<nbslns; j++){
    accxps[j] =  new fftw_complex[n2fft*pad/2+1];
  }

  fwd_plans = new fftw_plan[nstations];  
  
  //plan the FFTs
  if (RunPrms.get_messagelvl()> 0)
    cout << "\nPlanning the FFTs for correlation!\n";
  for (sn = 0; sn < nstations; sn++){
    fwd_plans[sn] =
      fftw_plan_dft_r2c_1d(n2fft*pad,invecs[sn],xps[sn],FFTW_EXHAUSTIVE);
  }

  //arrays and plans for delay correction
  sls  = new fftw_complex[lsegm];
  spls = new fftw_complex[lsegm];
  planBW = fftw_plan_dft_1d(lsegm, sls, spls, FFTW_BACKWARD, FFTW_ESTIMATE);
  planFW = fftw_plan_dft_1d(lsegm, spls, sls, FFTW_FORWARD,  FFTW_ESTIMATE);


  //arrays for delay tables
  delaydt = new INT64[nstations];
  tdel    = new INT64*[nstations];
  cdel    = new double*[nstations];
  mdel    = new double*[nstations];
  rdel    = new double*[nstations];
  fdel    = new double*[nstations];
  for (sn=0; sn<nstations; sn++) {
    //determine intervals in delay tables
    delaydt[sn] = Delaydt(StaPrms[sn].get_delaytable());
    ndel = sliceTime/delaydt[sn]+1;//nr of delay lines
    cout << "delaydt=" << delaydt[sn] << endl;
    cout << "sliceTime=" << sliceTime << endl;
    cout << "ndel=" << ndel << endl;
    cout << "sliceStartTime=" << sliceStartTime[core] << endl;
    tdel[sn]= new INT64[ndel];
    cdel[sn]= new double[ndel];
    mdel[sn]= new double[ndel];
    rdel[sn]= new double[ndel];
    fdel[sn]= new double[ndel];
    tdel[sn][0] = 0;
    ReadDelayTable(StaPrms[sn].get_delaytable(), sliceStartTime[core],
      ndel, GenPrms.get_cde(), GenPrms.get_mde(), GenPrms.get_rde(),
      tdel[sn], cdel[sn], mdel[sn], rdel[sn], fdel[sn]);
  }

  //open the output file
  outP = fopen64(GenPrms.get_corfile(),"wb");

  //open the input files
  //initialise file pointers in Bytes and set file pointers
  for (sn=0; sn<nstations; sn++) {
    inFile[sn]=open(StaPrms[sn].get_mk4file(),O_RDONLY,0);
    BytePtr[sn] = sliceStartByte[sn][core];
    statusPtr=lseek(inFile[sn],BytePtr[sn],SEEK_SET);
    if (statusPtr != BytePtr[sn]){
      cerr << "Could not go the requested file position for file: "<<
        StaPrms[sn].get_mk4file() << endl;
      return -1;
    }
  }
    

  //initialise dcBufPrev with data from Mk4 file
  for (sn=0; sn<nstations; sn++) {
    for (i=0; i<2*BufSize; i++) {
      if (FC[sn] == FL[sn]) {
        //fill Mk4frame if frame counter at end of array
        fill_Mk4frame(sn, inFile[sn], Mk4frame[sn], signST, magnST, Nsamp);
        FC[sn] = 0;
      }
      //fill remaining of dcBufs with data from Mk4
      dcBufPrev[sn][i]=Mk4frame[sn][FC[sn]];
      FC[sn]++;
      dcBufPrev[sn][i]=1;
    }
  }

  
  //loop initializations
  timePtr = sliceStartTime[core];
  BufPtr = BufSize;
  loop=0;
  if (RunPrms.get_messagelvl()> 0)
    cout << "Before timePtr=" << timePtr << endl;
  
  //while loop for processing from startbyte until stopbyte
  while (1)
  {
    loop++;
    if (RunPrms.get_messagelvl()> 0)
      cout << "loop=" << loop << endl;
    
    //initialise statistical values to zero
    for (sn=0; sn<nstations; sn++) {
      signST[sn]=0.0;
      magnST[sn]=0.0;
      Nsamp[sn]=0;
    }
  
    //zero accxps array and norms array
    for (i = 0; i < nbslns ; i++){
      for (j = 0 ; j < n2fft*pad/2+1; j++){
        accxps[i][j][0] = 0.0; //real 
        accxps[i][j][1] = 0.0; //imaginary
      }
      norms[i] = 0.0;
    }

    //process all the segments 
    for (segm = 0 ; segm < Nsegm2Avg ; segm++){

      //read data from data files, do pre-correlation, 
      //and put results in Bufs. 
      if ( (BufPtr+n2fft)>BufSize ) {
        fill_Bufs(inFile,
          Bufs,dcBufPrev,BufSize,
          Mk4frame,FL,FC,signST,magnST,Nsamp,
          sls,spls,planFW,planBW,
          tbs,fs,Nf,timePtr,
          delaydt,cdel,fdel,ndel,core);
        timePtr=timePtr+BufTime;
        BufPtr=0;
      }

      //get data from Bufs and put in invecs and increase BufPtr
      fetch_invecs(BufPtr, nstations, n2fft, invecs, Bufs);
      
      //correlations
      bsln = 0; //initialise basline number
      
      // FWD FFT each station +
      // form the auto products
      for (sn = 0 ; sn < nstations; sn++){
        //input: invecs -> result: xps
        fftw_execute(fwd_plans[sn]);
        for (l = 0 ; l < n2fft*pad/2 + 1 ; l++){
          //accxps[bsln][l] += xps[sn][l]*conj(xps[sn][l]);
          accxps[bsln][l][0] = accxps[bsln][l][0] +
          (xps[sn][l][0] * xps[sn][l][0]) + (xps[sn][l][1] * xps[sn][l][1]);
          //accxps[bsln][l][1] imaginary part stays zero
        }
        bsln++;
      }
            
      // form cross products
      for (sn = 0 ; sn < nstations - 1; sn++){
        for (sno = sn + 1; sno < nstations ; sno ++){
          for (l = 0 ; l < n2fft*pad/2 + 1 ; l++){
            //accxps[bsln][l] += xps[sn][l]*conj(xps[sno][l])
            accxps[bsln][l][0] = accxps[bsln][l][0] +
            (xps[sn][l][0] * xps[sno][l][0]) + (xps[sn][l][1] * xps[sno][l][1]);
            accxps[bsln][l][1] = accxps[bsln][l][1] +
            (xps[sn][l][1] * xps[sno][l][0]) - (xps[sn][l][0] * xps[sno][l][1]);
            
          }
          bsln++;
        }
      }
      
      if (segm%TenPct == 0)
        cout << "segm=" << segm << endl;
      
    }

    //average the sample statistics
    for (sn=0; sn<nstations; sn++) {
      signST[sn] = signST[sn]/Nsamp[sn];
      magnST[sn] = magnST[sn]/Nsamp[sn];
      if (RunPrms.get_messagelvl()> 0){
        cout << "Sample statistics " << "sn=" << sn <<
        " Nsamp=" << Nsamp[sn] <<
        " signST=" << signST[sn] <<
        " magnST=" << magnST[sn] << endl;
      }  
    }

    
    //average correlation results over all segments
    bsln = 0;//reinitialise baseline counter
    //auto product normalization, mean pwr = 1
    for (sn = 0 ; sn < nstations ; sn++){
      for (l = 0; l < n2fft*pad/2 + 1; l++){
        norms[bsln] = norms[bsln] + accxps[bsln][l][0];
      }
      norms[bsln] = norms[bsln] / (double)(n2fft*pad/2 + 1);
      for (l = 0; l < n2fft*pad/2 + 1; l++){
        accxps[bsln][l][0] = accxps[bsln][l][0] / norms[bsln];
      }
      bsln++;
    }
    //cross product normalization
    for (sn = 0 ; sn < nstations - 1; sn++){
      for (sno = sn + 1; sno < nstations ; sno ++){
        norms[bsln] = sqrt(norms[sn]*norms[sno]);
        for (l = 0 ; l < n2fft*pad/2 + 1 ; l++){
          accxps[bsln][l][0] = accxps[bsln][l][0] / norms[bsln];
          accxps[bsln][l][1] = accxps[bsln][l][1] / norms[bsln];
        }
        bsln++;
      }
    }

    //write normalized correlation results to output file
    for (bsln = 0; bsln < nbslns; bsln++){
      fwrite(accxps[bsln],sizeof(fftw_complex),n2fft*pad/2+1,outP);
    }

    //determine position of file read pointer 
    BytePtr[0] = lseek(inFile[0], 0, SEEK_CUR);
    if (RunPrms.get_messagelvl()> 0)
      cout <<
        "BytePtr[0]            =" << BytePtr[0] << endl <<
        "sliceStopByte[0]["<< core <<"]=" << sliceStopByte[0][core] << endl <<
        "timePtr=" << timePtr << endl;
    //break from while loop at end of time slice
    if (BytePtr[0] >= sliceStopByte[0][core]) break;
    
  } // End while loop for processing from startbyte until stopbyte
  
  if (RunPrms.get_messagelvl()> 0)
    cout << "After timePtr=" << timePtr << endl;
  //close output file

  //close the input files
  for (sn=0; sn<nstations; sn++){
    close(inFile[sn]);
  }
  //close the output result file
  fclose(outP);
  
  //free allocated memory   
  delete [] norms;
  delete [] inFile;
  delete [] BytePtr;
  delete [] signST;
  delete [] magnST;
  delete [] Nsamp;
  delete [] sls;
  delete [] spls;
  delete [] fs;
  fftw_destroy_plan(planFW);
  fftw_destroy_plan(planBW);
    
  for (sn=0; sn<nstations; sn++){
    delete [] Bufs[sn];
    delete [] dcBufPrev[sn];
  }
  delete [] Bufs;
  delete [] dcBufPrev;
  
  for (sn=0; sn<nstations; sn++)
    delete [] invecs[sn];
  delete [] invecs;
  
  delete [] FL;
  delete [] FC;
  for (sn=0; sn<nstations; sn++)
    delete [] Mk4frame[sn];
  delete [] Mk4frame;  

  
  for (sn=0; sn<nstations; sn++)
    delete [] xps[sn];
  delete [] xps;
  
  for (j=0; j<nbslns; j++)
    delete [] accxps[j];
  delete [] accxps;
  
  //destroy fftw plans
  for (sn=0; sn<nstations; sn++)
    fftw_destroy_plan(fwd_plans[sn]);

  delete [] delaydt;
  for (sn=0; sn<nstations; sn++){
    delete [] tdel[sn];
    delete [] cdel[sn];
    delete [] mdel[sn];
    delete [] rdel[sn];
    delete [] fdel[sn];
  }
  delete [] tdel;
  delete [] cdel;
  delete [] mdel;
  delete [] rdel;
  delete [] fdel;
  
  return 1;

}



//***************************************************************************
//read data from data files delay correction, and put results in Bufs.
//remember data in dcBufPrev
//***************************************************************************
int fill_Bufs(int *inFile,
  double **Bufs, double **dcBufPrev, int BufSize,
  double **Mk4frame, INT64 *FL, INT64 *FC,
  double *signST, double *magnST, INT64 *Nsamp,
  fftw_complex *sls, fftw_complex *spls, fftw_plan& planFW, fftw_plan& planBW,
  double tbs, double *fs, int Nf, double& timePtr,
  INT64 *delaydt, double **cdel, double **fdel, INT64 ndel, int core)
{
  //declarations
  int retval = 0;
  int i;
  int nstations, sn; //nr of stations and station counter
  int lsegm, jl; //fourier length of a segment in the pre-correlation
  double Lsegm; //same as lsegm but in float
  double sqrtLsegm;
  INT64 Nsegm2DC; //nr of segments to delay correct, result in Bufs
  INT64 jsegm; //segment number
  INT64 jshift; //address shift due to delay correction
  int   jf; // frequency element counter
  double **dcBufs; //buffers for delay corrections
  double Cdel,Fdel,Phase;
  double dfs, phi, FoffRatio;
  double Time; //time in micro seconds

  
  //initialisations and allocations
  nstations = GenPrms.get_nstations();
  lsegm = GenPrms.get_lsegm();
  Lsegm = lsegm;
  sqrtLsegm = sqrt(Lsegm);
  Nsegm2DC = BufSize/lsegm;
  Phase=0.0;
  
  dcBufs =new double*[nstations];
  for (sn=0; sn<nstations; sn++){
    dcBufs[sn] = new double[3*BufSize];
  }

  
  
  for (sn=0; sn<nstations; sn++){
  
    //fill dcBufs with data from dcBufPrev
    for (i=0; i<2*BufSize; i++) {
      dcBufs[sn][i]=dcBufPrev[sn][i];
    }
    
    for (i=2*BufSize; i<3*BufSize; i++) {
     if (FC[sn] == FL[sn]) {
        //fill Mk4frame if frame counter at end of array
        fill_Mk4frame(sn, inFile[sn], Mk4frame[sn], signST, magnST, Nsamp);
        FC[sn] = 0;
      }
      //fill remaining of dcBufs with data from Mk4file
      dcBufs[sn][i]=Mk4frame[sn][FC[sn]]; 
      FC[sn]++;
    }
    
    //apply delay and phase corrections for all segments
    //in other words process data in dcBufs, output in Bufs
    for (jsegm=0; jsegm<Nsegm2DC; jsegm++) {
      Time = timePtr + jsegm*lsegm*tbs*1000000.; //in usec    
      Cdel = ParInteRp(Time, sliceStartTime[core], cdel[sn], delaydt[sn], ndel);
      if (Cdel>0.0) {
        cerr << "Cdel > 0.0 in fill_Bufs." << endl;
        exit(0);
      }      
      //address shift due to time delay for the  current segment
      jshift = floor(Cdel/tbs+0.5);
      //fill the complex sls array
      for (jl=0; jl<lsegm; jl++){
        sls[jl][0] = dcBufs[sn][2*BufSize+jshift+jl+jsegm*lsegm];
        sls[jl][1] = 0.0;
      }

      //complex forward fourier transform
      fftw_execute(planBW);
      //apply normalization
      for (jl=0; jl<lsegm/2; jl++){
        spls[jl][0] = spls[jl][0] / sqrtLsegm;
        spls[jl][1] = spls[jl][1] / sqrtLsegm;
      }
      //zero the unused subband
      for (jl=lsegm/2;jl<lsegm;jl++){
        spls[jl][0] = 0.0;
        spls[jl][1] = 0.0;
      }
      //calculate the fract bit phase corrections and
      //apply them and pi/2 also
      Time = timePtr + jsegm*lsegm*tbs*1000000.+lsegm/2*tbs*1000000.;
      Cdel = ParInteRp(Time, sliceStartTime[core], cdel[sn], delaydt[sn], ndel);
      if (Cdel>0.0) {
        cerr << "Cdel > 0.0 in fill_Bufs." << endl;
        exit(0);
      }      
      dfs = Cdel/tbs - floor(Cdel/tbs + 0.5);
      FoffRatio=0.5+GenPrms.get_foffset()/GenPrms.get_bwfl();
      for (jf = 0; jf < Nf; jf++)
      {
        phi = -2.0*PI*dfs*tbs*fs[jf] + FoffRatio*PI*jshift/GenPrms.get_ovrfl();
        spls[jf][0] = spls[jf][0]*cos(phi)-spls[jf][1]*sin(phi);
        spls[jf][1] = spls[jf][0]*sin(phi)+spls[jf][1]*cos(phi);
      }
      //reverse complex fft
      fftw_execute(planFW);
      //apply normalization and multiply by 2.0
      for (jl=0; jl<lsegm; jl++){
        sls[jl][0] = sls[jl][0] / sqrtLsegm;
        sls[jl][1] = sls[jl][1] / sqrtLsegm;
      }
      //subtract dopplers and put real part in Bufs inside the segment
      for (jl=0;jl<lsegm;jl++)
      {
        Time = timePtr + jsegm*lsegm*tbs*1000000. + jl*tbs*1000000.;
        Fdel = ParInteRp(Time, sliceStartTime[core], fdel[sn], delaydt[sn], ndel);
//check with Sergei if Fdel has to be negative
//        if (Fdel>0.0) {
//          cerr << "Fdel > 0.0 in fill_Bufs." << endl;
//          exit(0);
//        }      
//Disabled for test purposes
//        if (phaseCorrOn) Phase = ParInteRp(Time, phase, delaydt);
        phi = -2.0*PI*(StaPrms[sn].get_loobs()+GenPrms.get_startf()+
          GenPrms.get_bwfl()*0.5+GenPrms.get_foffset())*Fdel + Phase;
        Bufs[sn][lsegm*jsegm+jl]=sls[jl][0]*sin(phi)-sls[jl][1]*cos(phi);
      }
      
    }
    
    //fill dcBufPrev arrays, remember for next loop
    for (i=0; i<2*BufSize; i++) {
      dcBufPrev[sn][i] = dcBufs[sn][BufSize+i];
    }
    
  }  
  

  //free allocated memory
  for (sn=0; sn<nstations; sn++)
    delete [] dcBufs[sn];
  delete [] dcBufs;
  
  return retval;
}



//*****************************************************************************
//fill invecs using the data from the buffer
//*****************************************************************************
int fetch_invecs(INT64& BufPtr, int nstations, int n2fft,
  double **invecs, double **Bufs)
{  
  int i,j;
  for (i = 0; i < nstations; i++){
    for (j = 0; j < n2fft; j++){
      invecs[i][j] = Bufs[i][j+ BufPtr];
    }
  }
  BufPtr=BufPtr+n2fft;
  return(0);
}


