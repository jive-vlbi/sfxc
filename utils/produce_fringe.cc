#include <iostream>
#include <complex>
#include <stdio.h>
#include <fstream>
#include <assert.h>
#include <fftw3.h>
using namespace std;

#define N 257
#define N2 512
#include <fstream>

int main(int argc, char *argv[])
{
  assert(argc == 2);

  // Example of a fourier transform
  complex<double> in[N2], out[N2];
  fftw_plan p;
  
  std::ifstream infile(argv[1], std::ios::in | std::ios::binary);
  assert(infile.is_open());

  p = fftw_plan_dft_1d(N2, 
                       reinterpret_cast<fftw_complex*>(&in),
                       reinterpret_cast<fftw_complex*>(&out),
                       FFTW_BACKWARD, 
                       FFTW_ESTIMATE);

  bool finished = false;
  int corr_nr=0;
  while (!finished) {
    char out_filename[20];
    sprintf(out_filename, "out%d.txt", corr_nr);
    corr_nr = (corr_nr+1)%3;
    std::ofstream fout(out_filename, std::ios::app);
    assert(fout.is_open());

    // read in one fourier segment
    for  (int i=0; i<N; i++) {
      infile.read((char *)&in[i], 2*sizeof(double));
    }
    for (int i=N; i<N2; i++) {
      in[i] = in[N2 - i];
    }

    // check whether we are finished
    finished = infile.eof();

    if (!finished) {
      fftw_execute(p); /* repeat as needed */
      
      for (int i=0; i<N; i++) {
        fout << in[i].real() << " \t" << in[i].imag() << " \t" 
             << out[i].real() << " \t" 
             << out[i].imag() << " \t" 
             << (i%N)
             << std::endl;
      }
    }
  }

  fftw_destroy_plan(p);
  
  return 0;
}
