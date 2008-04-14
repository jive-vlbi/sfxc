/*
author : RHJ Oerlemans
date   : 20070302
purpose: calculate fringes and perform fringe analysis using sfxc
         correlator product file as input data.
         use e.g. gnuplot to plot results in output files
*/
#include <iostream>
#include <complex>
#include <stdio.h>
#include <fstream>
#include <assert.h>
#include <fftw3.h>
#include <math.h>
#include <iomanip>
#include <sstream>
#include <string>
#include "utils.h"

using namespace std;


//function prototypes
int i_for_max(FLOAT *array, int length);
FLOAT noise_rms(complex<FLOAT> *array, int length, int imax);

//main
int main(int argc, char *argv[]) {
  int ns, nCross, nbslns, nAutos;

  if ((argc < 4) || (argc > 5)) {
    cout << "\nusage  : analyse_fringes  sfxc_product_filename  nr_of_stations  n2fft <ret>\n";
    cout << "usage  : analyse_fringes  sfxc_product_filename  nr_of_stations  n2fft r<ret>\n";
    cout << "purpose: calculate fringes from correlator product file and analyse the fringes.\n";
    cout << "output : fringes.txt, analysis?.txt. Where ? is cross-correlation baseline nr\n";
    cout << "         Use e.g. gnuplot for graphic presentation.\n";
    return 0;
  } else {

    ns     = atoi(argv[2]);//nr of stations
    nAutos = ns;

    if (argc == 4) {
      //all auto and cross correlations
      nCross = ns*(ns-1)/2;//nr of cross baselines
      nbslns = ns + ns*(ns-1)/2;//nr of baselines auto + cross
    } else {
      //one reference station: one auto and rest cross correltions
      nCross = ns-1;//nr of cross baselines
      nbslns = 2*ns-1;//nr of baselines auto + cross
    }

  }


  ifstream infile(argv[1], ios::in | ios::binary);
  assert(infile.is_open());

  int n2fft=atoi(argv[3]) + 1;//FFT length + 1 in correlation
  FLOAT ampl[n2fft]; //fringe amplitude


  //output file for auto and cross correlation fringes
  ofstream fout("fringes.txt");
  assert(fout.is_open());

  //write column header info in fringe output file
  fout << "# Correlator Product        Lag Product\n";
  fout << "#       Real         Imag         Real         Imag         Ampl\n";

  //output files for cross correlation analysis results
  string analysisName, s;
  ofstream Aout[nCross];
  for (int i=0; i<nCross; i++) {
    stringstream ss;
    ss << i;
    ss >> s;
    analysisName = "analysis" + s + ".txt";
    Aout[i].open(analysisName.c_str());
    assert(Aout[i].is_open());
    Aout[i] << "#  nT  bsln  iMax    amplMax      phase      noise        SNR    phase\n";
    Aout[i] << "#                                 radians                        degrees\n";
  }


  //frequency to lag FFT
  complex<FLOAT> in[n2fft], out[n2fft], outR[n2fft];
  FFTW_PLAN F2L;
  F2L = FFTW_PLAN_DFT_1D(n2fft,
                         reinterpret_cast<FFTW_COMPLEX*>(&in),
                         reinterpret_cast<FFTW_COMPLEX*>(&out),
                         FFTW_BACKWARD,
                         FFTW_ESTIMATE);


  //loop initialisations
  bool finished = false;
  int bsln = nbslns;
  int Cbsln = 0;
  int nT = 0;//initialise nr of time interval

  int lagNr=0;
  while (!finished) {
    if (bsln == nbslns) {
      bsln=0;
      Cbsln=0;
      nT++; //increase nr of time interval;
    }
    // read in one fourier segment
    for  (int i=0; i<n2fft; i++) {
      infile.read((char *)&in[i], 2*sizeof(FLOAT));
    }
    // check whether we are finished
    finished = infile.eof();

    if (!finished) {
      FFTW_EXECUTE(F2L);

      //fringe calculations write results to file
      for (int i=0; i<n2fft; i++) {
        outR[i] = out[(i+n2fft/2)%n2fft];
        ampl[i] = sqrt( outR[i].real() * outR[i].real() +
                        outR[i].imag() * outR[i].imag() );
        fout <<
        setw(12) << in[i].real() << " " <<
        setw(12) << in[i].imag() << " " <<
        setw(12) << outR[i].real() << " " <<
        setw(12) << outR[i].imag() << " " <<
        setw(12) << ampl[i] / n2fft << " " <<
        setw(12) << lagNr << endl;
        lagNr = (lagNr+1)%(nbslns*n2fft);
        if (lagNr == 0)  fout << endl;
      }

      //analyse cross correlation fringe and write results to file
      if (bsln >= nAutos) {
        //find lag for max amplitude
        int iForMax = i_for_max(ampl,n2fft);
        //calculate max amplitude
        FLOAT amplMax=ampl[iForMax];
        //calculate phase at max amplitude
        FLOAT argAmplMax=arg(outR[iForMax]);
        //calculate noise RMS
        FLOAT noiseRMS = noise_rms(outR,n2fft,iForMax);
        //signal noise ratio
        FLOAT SNR=amplMax/noiseRMS;
        //append to end of file, one output file per cross cor baseline
        Aout[Cbsln] <<
        setw(5) << nT << " " <<
        setw(5) << bsln << " " <<
        setw(5) << iForMax << " " <<
        setw(10) << amplMax / n2fft << " " <<
        setw(10) << argAmplMax << " " <<
        setw(10) << noiseRMS << " " <<
        setw(10) << SNR << " " <<
        setw(10) << argAmplMax/M_PI*180. << endl;
        Cbsln++;

      }
      bsln++;
    }

  }

  FFTW_DESTROY_PLAN(F2L);

  return 0;
}


//return array index for which value is max
int i_for_max(FLOAT *array, int length) {
  int    iForMax=0;
  // the following should be double
  FLOAT Max=array[iForMax];
  for (int i=1; i<length ; i++) {
    if (array[i] > Max) {
      iForMax=i;
      Max=array[iForMax];
    }
  }
  return iForMax;
}


//return noise rms in array, skip 10 % around maximum
FLOAT noise_rms(complex<FLOAT> *array, int length, int imax) {
  FLOAT noiseRMS;
  FLOAT meanR=0.0;
  FLOAT meanI=0.0;
  FLOAT sum=0.0;
  int ll=imax - length/20;//5% of range to left
  int ul=imax + length/20;//5% of range to right
  int n2avg=0;

  for (int i=0 ; i< length ; i++) {
    if ( (i < ll) || (i > ul) ) {
      //skip 10% arround lag for max which is at imax
      n2avg++;
      meanR += array[i].real();
      meanI += array[i].imag();
    }
  }

  meanR /= n2avg;
  meanI /= n2avg;

  for (int i=0 ; i< length ; i++) {
    if ((i < ll) || (i > ul)) {
      sum += pow( (array[i].real()-meanR),FLOAT(2.0) ) + pow( (array[i].imag()-meanI),FLOAT(2.0) );
    }
  }

  noiseRMS = sqrt(sum/n2avg);

  return noiseRMS;
}
