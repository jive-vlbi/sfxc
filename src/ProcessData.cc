/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Functions for processing the recorded data
- extract data from file
- optional filtering (not implemented yet)
- delay correction
- auto and cross- correlation

Author     : RHJ Oerlemans
StartDate  : 20061005
Last change: 20061114

*/

#include <types.h>

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
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
#include "delayTable.h"
#include "timer.h"

#include "Log_writer.h"
#include "Log_writer_void.h"

//global variables
extern RunP  RunPrms;
extern GenP  GenPrms;
extern StaP  StaPrms[NstationsMax];
//extern INT64 sliceStartByte[NstationsMax][NprocessesMax];
extern INT64 sliceStartTime [NprocessesMax];
extern INT64 sliceStopTime  [NprocessesMax];
//extern INT64 sliceTime;

extern UINT32 seed;

#include <Data_writer.h>

Data_writer *data_writer=NULL;
// NGHK: This variable should definately go into a correlate class:
void set_data_writer(Data_writer &writer) {
  if (data_writer!=NULL) delete(data_writer);
  data_writer = &writer;
}
Data_writer &get_data_writer() {
  assert(data_writer != NULL);
  return *data_writer;
}

// NGHK: This variable should definately go into a correlate class:
Log_writer      *log_writer=NULL;
void set_log_writer(Log_writer &writer) {
  if (log_writer!=NULL) delete(log_writer);
  log_writer = &writer;
}
Log_writer &get_log_writer() {
  assert(log_writer != NULL);
  return *log_writer;
}


//***************************************************************************
//prototypes for local functions
//***************************************************************************
int fill_Bufs(std::vector<Data_reader *> &readers,
  double **Bufs, double **dcBufPrev, int BufSize,
  double **Mk4frame, INT64 *FL, INT64 *FC,
  double *signST, double *magnST, INT64 *Nsamp,
  fftw_complex *sls, fftw_complex *spls, fftw_plan& planFW, fftw_plan& planBW,
  double tbs, double *fs, int Nf, double timePtr,
  std::vector<DelayTable> &delTbl);

int fetch_invecs(INT64& BufPtr, int nstations, int n2fft,
  double **invecs, double **Bufs);


// NGHK: Make this global for a single initialisation during the correlation process, 
//       put in a correlate class later
bool init_delTbl = true;
std::vector<DelayTable> delTbl;
int initialise_delay_tables(int nstations, StaP StaPrms[]) {
  if (!init_delTbl) return 0;
  get_log_writer().message(1,"initialise_delay_tables");
  init_delTbl = false;
  
  delTbl.resize(nstations);
  for (int sn=0; sn<nstations; sn++) {
    string msg = string("DelTbl: ")+StaPrms[sn].get_delaytable();
    get_log_writer().message(2,msg);
    int retval = delTbl[sn].readDelayTable(StaPrms[sn].get_delaytable(), BufTime );
    if (retval != 0) {
      get_log_writer().message(0,"ERROR: when reading delay table.\n");
      return retval;
    }
  }
  return 0;
}


void correlation_add_delay_table(DelayTable &table) {
  get_log_writer().message(1,"correlation_add_delay_table");
  init_delTbl = false;
  
  delTbl.push_back(table);
  assert(table == delTbl.back());
}

//declarations
int retval = 0;
int i,j,l;
int nstations, sn, sno; //nr of stations and station counters
int n2fft; //FFT length in correlation
int pad; //padding with zeros for invecs
INT64 Nsamp2Avg; //nr of samples to average the correlation
INT64 Nsegm2Avg; //nr of fourier segments to average the correlation
INT64 segm; //segment number in for loop
INT64 TenPct;
INT64 BufPtr; //location of the buffer pointer
INT64 *Nsamp = NULL;
double timePtr; //a time pointer in micro seconds
int nbslns; //number of baselines, cross and auto correlations
int bsln; // baseline number
float *norms = NULL; //normalization coeffs
double **Bufs = NULL; //buffers with pre-correlated data ready for correlation
double **dcBufPrev = NULL; //buffers with data for delay correction
int BufSize; // size of one buffer in Bufs
double **invecs = NULL;//input vectors for FFT operation
fftw_complex **xps = NULL; //xps: result vectors from FFT
//fftw_complex **accxps = NULL; //accumulated cross and auto powers Real and Imaginary part
fftw_plan *fwd_plans = NULL; //FFT plans
double **Mk4frame = NULL; //channel data for one frame
INT64 *FL = NULL; //Mk4 frame length
INT64 *FC = NULL; //Mk4 frame counter
double *signST = NULL, *magnST = NULL;//sign and magnitude statistics
INT64 loop; //while loop counter
double SR, tbs; //sample rate and time between samples
double dfr, *fs = NULL; // delta frequency in frequency scale, frequency scale
int    jf, Nf; //nr of frequencies in frequency scale
char outFile[256], coreStr[5];
//timer parameters
ptimer  tmr1;
// accumulated cross and auto powers Real and Imaginary part
fftw_complex **accxps;

//declarations for fftw in delay correction
fftw_complex *sls = NULL, *spls = NULL; //FW:in,out; BW: out,in
fftw_plan    planFW, planBW;//plans for forward and backward fft 
int lsegm; //fourier length of a segment in the pre-correlation

std::vector<Data_reader *> readers;

int CorrelateBufs_initialise(std::vector<Data_reader *> &readers_) {
  // All variables needed for CorrelateBufs
  seed = (UINT32) time((time_t *)NULL);
  seed = 10;
  {
    std::stringstream msg;
    msg << "CorrelateBufs: seed: " << seed;
    get_log_writer().warning(msg);
  }

  readers = readers_;

  tmr1 = (ptimer)malloc(sizeof(timer));

  get_log_writer()(0) << "Correlation process started." << std::endl;
  
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
  BufSize   = BufTime * (int)(SR/1000000.0);
  Nf = lsegm/2+1; //number of frequencies
  dfr = 1.0/(lsegm*tbs); // delta frequency
  fs = new double[Nf]; // frequency array
  for (jf=0; jf<Nf; jf++) //frequency scale in the segment
    fs[jf]=jf*dfr-0.5*GenPrms.get_bwfl()-GenPrms.get_foffset();

  {
    get_log_writer()(2)
      << "BufSize=" << BufSize << endl
      << "Nsamp2Avg=" << Nsamp2Avg << endl
      << "n2fft    =" << n2fft << endl
      << "Nsegm2Avg=" << Nsegm2Avg << endl
      << "SR  = " << SR << endl
      << "tbs = " << tbs << endl
      << "Nf  = " << Nf << endl
      << "dfr = " << dfr << endl;
    
  }
    
  norms = new float[nbslns];
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
  FL = new INT64 [nstations];//frame lengths
  FC = new INT64 [nstations];//frame counters

  for (sn=0; sn<nstations; sn++){
    FL[sn] = frameMk4*StaPrms[sn].get_fo();//initialise
    FC[sn] = FL[sn];//set counter to end of frame
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
  get_log_writer().message(1,"\nPlanning the FFTs for correlation!\n");
  for (sn = 0; sn < nstations; sn++){
    fwd_plans[sn] =
      fftw_plan_dft_r2c_1d(n2fft*pad,invecs[sn],xps[sn],FFTW_EXHAUSTIVE);
  }  

  //arrays and plans for delay correction
  sls   = new fftw_complex[lsegm];
  spls  = new fftw_complex[lsegm];
  planBW = fftw_plan_dft_1d(lsegm, sls, spls, FFTW_BACKWARD, FFTW_ESTIMATE);
  planFW = fftw_plan_dft_1d(lsegm, spls, sls, FFTW_FORWARD,  FFTW_ESTIMATE);

  retval = initialise_delay_tables(nstations, StaPrms);
  if (retval != 0) {
    return retval;
  }
  assert((int)delTbl.size() == nstations);

  //initialise dcBufPrev with data from Mk4 file
  for (sn=0; sn<nstations; sn++) {
    for (i=0; i<2*BufSize; i++) {
      if (FC[sn] == FL[sn]) {
        //fill Mk4frame if frame counter at end of frame
        fill_Mk4frame(sn,*readers[sn],Mk4frame,signST,magnST,Nsamp);
        FC[sn] = 0;
      }
      dcBufPrev[sn][i]=Mk4frame[sn][FC[sn]];
      FC[sn]++;
    }
  }
                                                                 
  //loop initializations
  timePtr = GenPrms.get_usEarliest();
  BufPtr = BufSize;
  loop=0;

  
  if (get_log_writer().get_messagelevel()> 0) {
    get_log_writer()(1)
      << "Starting correlation: " << timePtr << " bufptr: " << BufPtr << std::endl;

    char sB1[25];
    strcpy(sB1,"Correlation");
    strcpy(tmr1->ID,sB1);
    tmrBegin(tmr1,get_log_writer());
  }

  return retval;
}

int CorrelateBufs_process_segment() {
  loop++;
  get_log_writer()(1) << "Process loop=" << loop << endl;;
  
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
      retval = fill_Bufs(readers,Bufs,dcBufPrev,BufSize,
        Mk4frame,FL,FC,signST,magnST,Nsamp,sls,spls,planFW,planBW,
        tbs,fs,Nf,timePtr,delTbl);
      if (retval !=0) {
        get_log_writer().message(1, "ERROR: in function fill_Bufs\n");
        return retval;
      }
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
          (xps[sn][l][0] * xps[sno][l][0]) +
          (xps[sn][l][1] * xps[sno][l][1]);
          
          accxps[bsln][l][1] = accxps[bsln][l][1] +
          (xps[sn][l][1] * xps[sno][l][0]) -
          (xps[sn][l][0] * xps[sno][l][1]);
          
        }
        bsln++;
      }
    }
    
    if (get_log_writer().get_messagelevel()> 0) {
      if (segm%TenPct == 0) {
        get_log_writer()(2) << "segm=" << segm << std::endl;
      }  
    }    
    
  }

  //average the sample statistics
  for (sn=0; sn<nstations; sn++) {
    signST[sn] = signST[sn]/Nsamp[sn];
    magnST[sn] = magnST[sn]/Nsamp[sn];
    if (get_log_writer().get_messagelevel()> 0){
      get_log_writer()(1)
        << "Sample statistics " << "sn=" << sn 
        << " Nsamp=" << Nsamp[sn] 
        << " signST=" << signST[sn] 
        << " magnST=" << magnST[sn] << endl;
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
  // NGHK: Make arrays consecutive to be able to write all data at once
  for (bsln = 0; bsln < nbslns; bsln++){
    //fwrite(accxps[bsln],sizeof(fftw_complex),n2fft*pad/2+1,outP);
    get_data_writer().put_bytes(sizeof(fftw_complex)*(n2fft*pad/2+1),
                                (char *)(accxps[bsln]));
  }

  // Check wether we are finished.
  get_log_writer()(1) << "TimePtr = " << (INT64)timePtr << std::endl;
  get_log_writer()(1) << "Stoptime= " << GenPrms.get_usStop() << std::endl;
  
  if (timePtr > GenPrms.get_usStop()) {
    get_log_writer()(1) << "Finished, timePtr after stopTime" << std::endl;
    return 1; //
  }
  return 0;
}

int CorrelateBufs_finalise() {
  tmrEnd(tmr1,get_log_writer());
  
  //free allocated memory   
  
  fftw_destroy_plan(planFW);
  fftw_destroy_plan(planBW);
  
  delete [] spls;
  delete [] sls;

  for (sn=0; sn<nstations; sn++)
    fftw_destroy_plan(fwd_plans[sn]);
  delete [] fwd_plans;
  
  for (j=0; j<nbslns; j++)
    delete [] accxps[j];
  delete [] accxps;
  
  for (sn=0; sn<nstations; sn++)
    delete [] xps[sn];
  delete [] xps;
  
  for (sn=0; sn<nstations; sn++)
    delete [] Mk4frame[sn];
  delete [] Mk4frame;  
  delete [] FL;
  delete [] FC;

  for (sn=0; sn<nstations; sn++)
    delete [] invecs[sn];
  delete [] invecs;
  
  for (sn=0; sn<nstations; sn++){
    delete [] Bufs[sn];
    delete [] dcBufPrev[sn];
  }
  delete [] Bufs;
  delete [] dcBufPrev;
  
  delete [] Nsamp;
  delete [] magnST;
  delete [] signST;
  delete [] norms;
  
  delete [] fs;
  
  get_log_writer()(0) << "Correlation finished." << std::endl;
  
  return 0;
}
  
//***************************************************************************
// fill the buffers with pre-correlation data and correlate
//***************************************************************************
int CorrelateBufs(std::vector<Data_reader *> &readers)
{
  int retval;
  retval = CorrelateBufs_initialise(readers);

  if (retval < 0) return retval;
  
  //while loop for processing from starttime until stoptime
  while ((retval=CorrelateBufs_process_segment())==0)
  { } // End while loop for processing from startbyte until stopbyte

  if (retval < 0) return retval;

  return CorrelateBufs_finalise();
}



//***************************************************************************
//read data from data files delay correction, and put results in Bufs.
//remember data in dcBufPrev
//***************************************************************************
int fill_Bufs(std::vector<Data_reader *> &readers,
  double **Bufs, double **dcBufPrev, int BufSize,
  double **Mk4frame, INT64 *FL, INT64 *FC,
  double *signST, double *magnST, INT64 *Nsamp,
  fftw_complex *sls, fftw_complex *spls, fftw_plan& planFW, fftw_plan& planBW,
  double tbs, double *fs, int Nf, double timePtr,
  std::vector<DelayTable> &delTbl)
{
  //declarations
  int retval = 0;
  int i;
  int nstations, sn; //nr of stations and station counter
  int lsegm,lsegm2, jl; //fourier length of a segment in the pre-correlation
  double Lsegm; //same as lsegm but in float
  double sqrtLsegm;
  INT64 Nsegm2DC; //nr of segments to delay correct, result in Bufs
  INT64 jsegm; //segment number
  INT64 jshift = 0; //address shift due to delay correction
  int   jf; // frequency element counter
  double **dcBufs = NULL; //buffers for delay corrections
  double Cdel,Fdel,Phase;
  double dfs, phi, FoffRatio;
  double Time; //time in micro seconds

  //initialisations and allocations
  nstations = GenPrms.get_nstations();
  lsegm = GenPrms.get_lsegm();
  lsegm2= lsegm/2;
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
        //fill Mk4frame if FrameCounter is at end of frame
        int nBytes =
        fill_Mk4frame(sn,*readers[sn],Mk4frame,signST,magnST,Nsamp);
        if (nBytes==0) {
          cerr << "ERROR: End of input for reader " << sn << endl;
          return 1;
        }
        FC[sn] = 0;//reset FrameCounter for station sn 
      }
      //fill remaining of dcBufs with data from Mk4file
      dcBufs[sn][i]=Mk4frame[sn][FC[sn]];
      FC[sn]++;
    }

    
    //apply delay and phase corrections for all segments
    //in other words process data in dcBufs, output in Bufs
    
    for (jsegm=0; jsegm<Nsegm2DC; jsegm++) {
    
      Time = timePtr + jsegm*lsegm*tbs*1000000.; //in usec
      Cdel = delTbl[sn].calcDelay(Time, DelayTable::Cdel);
      if (Cdel>0.0) {
        cerr << "Cdel > 0.0 in fill_Bufs()." << endl;
        return 1;
      }      
      //address shift due to time delay for the  current segment
      jshift = (INT64)(Cdel/tbs+0.5);
      
      //fill the complex sls array
      for (jl=0; jl<lsegm; jl++){
        sls[jl][0] = dcBufs[sn][2*BufSize + jsegm*lsegm + jl + jshift];
        sls[jl][1] = 0.0;
      }

      //complex forward fourier transform
      fftw_execute(planBW);
      //apply normalization
      for (jl=0; jl<lsegm; jl++){
        spls[jl][0] = spls[jl][0] / sqrtLsegm;
        spls[jl][1] = spls[jl][1] / sqrtLsegm;
      }
      
      //multiply element 0 and lsegm2 by 0.5
      //to avoid jumps at segment borders
      spls[0][0]=0.5*spls[0][0];//DC
      spls[0][1]=0.5*spls[0][1];
      spls[lsegm2][0]=0.5*spls[lsegm2][0];//Nyquist
      spls[lsegm2][1]=0.5*spls[lsegm2][1];
      //zero the unused subband
      for (jl=lsegm2+1;jl<lsegm;jl++){
        spls[jl][0] = 0.0;
        spls[jl][1] = 0.0;
      }
      //calculate the fract bit phase corrections and
      //apply them and pi/2 also
      Time = timePtr + jsegm*lsegm*tbs*1000000.+lsegm/2*tbs*1000000.;
      Cdel = delTbl[sn].calcDelay(Time, DelayTable::Cdel);
      if (Cdel>0.0) {
        cerr << "Cdel > 0.0 in fill_Bufs()." << endl;
        return 1;
      }      
      dfs = Cdel/tbs - floor(Cdel/tbs + 0.5);
      FoffRatio=0.5+GenPrms.get_foffset()/GenPrms.get_bwfl();
      
      for (jf = 0; jf < Nf; jf++)
      {
        phi = -2.0*M_PI*dfs*tbs*fs[jf] + FoffRatio*M_PI*jshift/GenPrms.get_ovrfl();
        double tmpR = spls[jf][0];
        double tmpI = spls[jf][1];
        spls[jf][0] = tmpR*cos(phi)-tmpI*sin(phi);
        spls[jf][1] = tmpR*sin(phi)+tmpI*cos(phi);
      }
      
      //reverse complex fft
      fftw_execute(planFW);
      
      //apply normalization and multiply by 2.0
      for (jl=0; jl<lsegm; jl++){
        sls[jl][0] = 2.0*sls[jl][0] / sqrtLsegm;
        sls[jl][1] = 2.0*sls[jl][1] / sqrtLsegm;//not used
      }

      //subtract dopplers and put real part in Bufs inside the segment
      for (jl=0;jl<lsegm;jl++)
      {
      
        Time = timePtr + jsegm*lsegm*tbs*1000000. + jl*tbs*1000000.;
        Fdel = delTbl[sn].calcDelay(Time, DelayTable::Fdel);
        if (Fdel>0.0) {
          cerr << "Fdel > 0.0 in fill_Bufs()." << endl;
          return 1;
        }      
       //TODO not implemented
       //   if (phaseCorrOn) Phase = phsTbl[sn].calcPhase(Time);
        phi = -2.0*M_PI*(StaPrms[sn].get_loobs()+GenPrms.get_startf()+
          GenPrms.get_bwfl()*0.5+GenPrms.get_foffset())*Fdel + Phase;
        Bufs[sn][lsegm*jsegm+jl]=sls[jl][0]*cos(phi)-sls[jl][1]*sin(phi);

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

