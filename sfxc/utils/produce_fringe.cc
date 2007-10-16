#include <iostream>
#include <complex>
#include <stdio.h>
#include <fstream>
#include <assert.h>
#include <fftw3.h>
#include <math.h>
using namespace std;

#define N 1024
#include <fstream>

int main(int argc, char *argv[])
{
  assert(argc == 2);

  // Example of a fourier transform
  complex<double> in[N+1], out[N+1];
  fftw_plan p;
  
  std::ifstream infile(argv[1], std::ios::in | std::ios::binary);
  assert(infile.is_open());

  std::ofstream fout("out.txt");
  assert(fout.is_open());

  p = fftw_plan_dft_1d(N+1, 
                       reinterpret_cast<fftw_complex*>(&in),
                       reinterpret_cast<fftw_complex*>(&out),
                       FFTW_BACKWARD, 
                       FFTW_ESTIMATE);

  bool finished = false;
  while (!finished) {
    // read in one fourier segment
    for  (int i=0; i<N+1; i++) {
      infile.read((char *)&in[i], 2*sizeof(double));
    }
    // check whether we are finished
    finished = infile.eof();

    if (!finished) {
      fftw_execute(p); /* repeat as needed */
      
      for (int i=0; i<N+1; i++) {
        fout 
          << in[i].real() << " \t" << in[i].imag() << " \t" 
          << out[(i+(N+1)/2)%(N+1)].real() << " \t" << out[(i+(N+1)/2)%(N+1)].imag() << " \t" 
          << sqrt(out[(i+(N+1)/2)%(N+1)].real()*out[(i+(N+1)/2)%(N+1)].real() + 
             out[(i+(N+1)/2)%(N+1)].imag()*out[(i+(N+1)/2)%(N+1)].imag()) << " \t"
          << std::endl;
      }
    }
  }

  fftw_destroy_plan(p);
  
  return 0;
}
