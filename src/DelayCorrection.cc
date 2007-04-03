/*
Author     : RHJ Oerlemans
StartDate  : 20070201
Last change: 20070209
*/

#include "DelayCorrection.h"


//Allocate arrays, initialise parameters
DelayCorrection::DelayCorrection(Log_writer &lg_wrtr)
  : log_writer(lg_wrtr)
{
}
//Allocate arrays, initialise parameters
DelayCorrection::DelayCorrection(GenP &GenPrms_, 
                                 StaP *StaPrms_, 
                                 Log_writer &lg_wrtr)
  : log_writer(lg_wrtr)
{
  set_parameters(GenPrms_, StaPrms_);
}

//Allocate arrays, initialise parameters
void DelayCorrection::set_parameters(GenP &GenPrms_, StaP *StaPrms_)
{
  GenPrms     = GenPrms_;
  StaPrms     = StaPrms_;

  nstations   = GenPrms.get_nstations();
  n2fftDC     = GenPrms.get_lsegm();
  SR          = 2.0*GenPrms.get_bwfl()*GenPrms.get_ovrfl();
  BufSize     = BufTime * (int)(SR/1000000.0);
  tbs         = 1.0/SR;
  Nsegm2DC    = BufSize/n2fftDC;

  Nf          = n2fftDC/2+1; //number of frequencies
  double dfr  = 1.0/(n2fftDC*tbs); // delta frequency
  fs          = new double[Nf]; // frequency array
  for (int jf=0; jf<Nf; jf++) //frequency scale in the segment
    fs[jf]=jf*dfr-0.5*GenPrms.get_bwfl()-GenPrms.get_foffset();

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
  sls   = new fftw_complex[n2fftDC];
  spls  = new fftw_complex[n2fftDC];
  planT2F = fftw_plan_dft_1d(n2fftDC, sls, spls, FFTW_BACKWARD, FFTW_ESTIMATE);
  planF2T = fftw_plan_dft_1d(n2fftDC, spls, sls, FFTW_FORWARD,  FFTW_ESTIMATE);
  //TODO RHJO: ask SP why not use fftw_plan_r2c_1d and fftw_plan_c2r_1d. Try!
  //4b) and 4c) probably not necessary anymore


  //set vector sizes
  delTbl.resize(nstations);//delay tables 
  data_reader.resize(nstations,NULL);//data_readers


  data_frame = new double *[nstations];
  df_length = new INT32 [nstations];//frame lengths
  df_counter = new INT32 [nstations];//frame counters

  for (int sn=0; sn<nstations; sn++){
    //TODO RHJO add datatype check for other data types
    df_length[sn] = frameMk4*StaPrms[sn].get_fo();//initialise
    df_counter[sn] = df_length[sn];//set counter to end of frame
    data_frame[sn] = new double[df_length[sn]];
  }
  

}


//De-allocate arrays and destroy plans
DelayCorrection::~DelayCorrection()
{

  delete [] fs;

  for (int sn=0; sn<nstations; sn++){
    delete [] segm[sn];
    delete [] Bufs[sn];
    delete [] dcBufs[sn];
    delete [] data_frame[sn];
  }
  delete [] segm;  
  delete [] Bufs;
  delete [] dcBufs;
  delete [] df_length;
  delete [] df_counter;

  fftw_destroy_plan(planF2T);
  fftw_destroy_plan(planT2F);
  
  delete [] spls;
  delete [] sls;

}



//set local data reader parameter
void DelayCorrection::set_data_reader(int sn, Data_reader *data_reader_)
{
  data_reader[sn]=data_reader_;
}



//go to desired position in input reader for station sn
void DelayCorrection::init_reader(int sn, INT64 startIS)
{
  
  int   jsynch; //shift to go to the synch word in the buffer
  INT64 usTime; //time stamp of current header wrt start of day (usec)
  INT64 offTime; //offset time (usec) wrt current header
  INT64 offFrames; //offset in frames wrt current header
  INT64 offSynch; //offset (bytes) caused bty jsynch
  INT64 offBytes; //total offset (bytes) wrt current reader position

  int msglvl; //message level parameter
  
  
  if (StaPrms[sn].get_datatype() == DATATYPE_MK4)
  {

    // remember message level and set message level to 0
    msglvl = get_log_writer().get_messagelevel();
    get_log_writer().set_messagelevel(0);
    // return usTime and jsynch for current header
    FindHeaderMk4(*data_reader[sn], jsynch, usTime, startIS, StaPrms[sn], GenPrms);
    // reset message level
    get_log_writer().set_messagelevel(msglvl);
    
    //calculate offsets 
    offTime = startIS - usTime;
    offFrames = offTime * StaPrms[sn].get_tbr()/frameMk4;
    if (StaPrms[sn].get_nhs() == 1) {
      offBytes = offFrames * frameMk4 * 4;
      offSynch = (jsynch -64) *4;
     } else {
      offBytes = offFrames * frameMk4 * 8;
      offSynch = (jsynch -64) *8;
    }
    offBytes = StaPrms[sn].get_boff() + offBytes + offSynch;
    //go to desired position in input_reader by moving offBytes forward
    data_reader[sn]->get_bytes(offBytes, NULL);
  }
  else
  {
    get_log_writer().message(0,"Unknown data type");
    assert(false);
  }
    
  //initialise dcBufPrev with data from input channel (can be Mk4 file)
  for (int i=0; i<2*BufSize; i++) {
    if (df_counter[sn] == df_length[sn]) {
      //fill data_frame if data frame counter at end of frame
      //TODO RHJO implement data type check for other data type

      fill_Mk4frame(sn,*data_reader[sn],data_frame, StaPrms[sn]);
      df_counter[sn] = 0;
    }
    dcBufPrev[sn][i]=data_frame[sn][df_counter[sn]];
    df_counter[sn]++;
  }
  
}


// fills the next segment to be processed by correlator core.
void DelayCorrection::fill_segment()
{
  //(re)fill Bufs when all data in Bufs is processed
  if ( (BufPtr + n2fftcorr) > BufSize ) {
    fill_Bufs();
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
}


// returns pointer to segment with delay corrected data.
double **DelayCorrection::get_segment()
{
  return segm;
}


//Fills Bufs with delay corrected data. This function has to be called
//every time all data in Bufs are processed.
void DelayCorrection::fill_Bufs()
{
//  double Time; //time in micro seconds
  INT64  Time; //time in micro seconds
  double Cdel;
  double Fdel; 
  double Phase;
  double phi;
  double tmpR;
  double tmpI;
  double sqrtN2fft = sqrt((double) n2fftDC);
  double dfs;      
  double FoffRatio;

  int jshift; //address shift due to signal delay wrt Earth center
  
  for (int sn=0; sn<nstations; sn++){
  
    //fill part 1 and 2 of dcBufs with data from dcBufPrev
    for (int i=0; i<2*BufSize; i++) dcBufs[sn][i]=dcBufPrev[sn][i];
    
    //fill part 3 of dcBufs with data from input channel (can be Mk4File)
    for (int i=2*BufSize; i<3*BufSize; i++) {
      if (df_counter[sn] == df_length[sn]) {
        //fill data frame if data frame counter is at end of frame
        int nBytes = fill_Mk4frame(sn,*data_reader[sn],data_frame,StaPrms[sn]);
        if (nBytes==0) {
//TODO RHJO implement error handling
//          cerr << "ERROR: End of input for reader " << sn << endl;
//          return 1;
        }
        df_counter[sn] = 0;//reset FrameCounter for station sn
      }
      //fill remaining of dcBufs with data from Mk4file
      dcBufs[sn][i]=data_frame[sn][df_counter[sn]];
      df_counter[sn]++;
    }
    
    //apply delay and phase corrections for all segments (n2fftDC long)
    //in other words process data in dcBufs, output in Bufs
    for (int jsegm=0; jsegm<Nsegm2DC; jsegm++) {


      Time = timePtr + (INT64)(jsegm*n2fftDC*tbs*1000000); //micro sec 
      Cdel = delTbl[sn].calcDelay(Time, DelayTable::Cdel);
      
      // 1)calculate the address shift due to time delay for the current segment
      jshift = (INT64)(Cdel/tbs+0.5);
      
      // 2)apply the address shift when filling the complex sls array
      for (int jl=0; jl<n2fftDC; jl++){
        sls[jl][0] = dcBufs[sn][2*BufSize + jsegm*n2fftDC + jl + jshift];
        sls[jl][1] = 0.0;
      }

      // 3) execute the complex to complex FFT, from Time to Frequency domain
      //    input: sls. output spls
      fftw_execute(planT2F);
      
      // 4a)apply normalization
      for (int jl=0; jl<n2fftDC; jl++){//TODO RHJO replace upper limit by n2fftDC/2+1
        spls[jl][0] = spls[jl][0] / sqrtN2fft;
        spls[jl][1] = spls[jl][1] / sqrtN2fft;
      }
      
      // 4b)multiply element 0 and n2fftDC/2 by 0.5
      //    to avoid jumps at segment borders
      spls[0][0]=0.5*spls[0][0];
      spls[0][1]=0.5*spls[0][1];
      spls[n2fftDC/2][0]=0.5*spls[n2fftDC/2][0];//Nyquist
      spls[n2fftDC/2][1]=0.5*spls[n2fftDC/2][1];
      
      // 4c) zero the unused subband
      for (int jl=n2fftDC/2+1;jl<n2fftDC;jl++){
        spls[jl][0] = 0.0;
        spls[jl][1] = 0.0;
      }

      
      // 5a)calculate the fract bit shift (=phase corrections in freq domain)
      Time = timePtr + (INT64)(jsegm*n2fftDC*tbs*1000000 + n2fftDC/2*tbs*1000000);
      Cdel = delTbl[sn].calcDelay(Time, DelayTable::Cdel);
      dfs  = Cdel/tbs - floor(Cdel/tbs + 0.5);
      FoffRatio=0.5+foffset/bwfl;//TODO RHJO foffset=0 replace FoffRatio by 0.5

      // 5b)apply phase correction in frequency range
      for (int jf = 0; jf < Nf; jf++){
        phi  = -2.0*M_PI*dfs*tbs*fs[jf] + FoffRatio*M_PI*jshift/ovrfl;
        tmpR = spls[jf][0];
        tmpI = spls[jf][1];
        spls[jf][0] = tmpR*cos(phi)-tmpI*sin(phi);
        spls[jf][1] = tmpR*sin(phi)+tmpI*cos(phi);
      }
      
      // 6a)execute the complex to complex FFT, from Frequency to Time domain
      //    input: spls. output sls
      fftw_execute(planF2T);
      
      // 6b)apply normalization and multiply by 2.0
      for (int jl=0; jl<n2fftDC; jl++){
        sls[jl][0] = 2.0*sls[jl][0] / sqrtN2fft;
        sls[jl][1] = 2.0*sls[jl][1] / sqrtN2fft;//not used
      }

      // 7)subtract dopplers and put real part in Bufs for the current segment
      for (int jl=0;jl<n2fftDC;jl++) {

        Time = timePtr + (INT64)(jsegm*n2fftDC*tbs*1000000 + jl*tbs*1000000);
        Fdel = delTbl[sn].calcDelay(Time, DelayTable::Fdel);
        Phase=0.0; //TODO RHJO phase correction to be implemented later
        phi  =-2.0*M_PI*Fdel*(skyfreq + startf + bwfl*0.5)+Phase;

        Bufs[sn][n2fftDC*jsegm+jl]=sls[jl][0]*cos(phi)-sls[jl][1]*sin(phi);
      }
      
    
    }
    
    //fill dcBufsPrev with part 2 and 3 from dcBufs.
    //in other words: remember for filling the next Bufs
    for (int i=0; i<2*BufSize; i++) 
      dcBufPrev[sn][i] = dcBufs[sn][BufSize+i];

  }
}


Log_writer& DelayCorrection::get_log_writer()
{
  return log_writer;
}

//set local delay table parameter
void DelayCorrection::set_delay_table(int sn, DelayTable &delay_table)
{
  delTbl[sn]=delay_table;
}

