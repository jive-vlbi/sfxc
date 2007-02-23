/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: test_Correlator_node.cc 144 2007-01-31 07:34:21Z kruithof $
 */
#include <iostream>
#include <complex>
#include <stdio.h>
#include <fstream>
#include <assert.h>
#include <fftw3.h>
using namespace std;

#define N2 1024
#define N  (N2/2 + 1)

// Number of output files:
#define NFILES 1
// Number of subsequent integration steps (fourier series) in one file
#define NINTERGRATIONS_IN_ONE_FILE 3

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

  for (int i=0; i<NFILES; i++) {
    char out_filename[20];
    sprintf(out_filename, "out%d.txt", i);
    unlink(out_filename);
  }


  bool finished = false;
  int corr_nr=0;
  while (!finished) {
    char out_filename[20];
    sprintf(out_filename, "out%d.txt", corr_nr);
    corr_nr = (corr_nr+1)%NFILES;
    std::ofstream fout(out_filename, std::ios::app);
    assert(fout.is_open());

    for (int integration=0; 
         (!finished) && (integration!=NINTERGRATIONS_IN_ONE_FILE); 
         integration++) {
      // read in one fourier segment
      for  (int i=0; i<N; i++) {
        infile.read((char *)&in[N-1-i], 2*sizeof(double));
      }
      for (int i=N; i<N2; i++) {
        in[i] = in[N2 - i];
      }
  
      // check whether we are finished
      finished = infile.eof();
  
      if (!finished) {
        fftw_execute(p); /* repeat as needed */
        
        for (int i=0; i<N; i++) {
          fout << integration*N+i << " \t"
               << in[i].real() << " \t" 
               << in[i].imag() << " \t" 
               << out[i].real() << " \t" 
               << out[i].imag() << " \t" 
               << std::endl;
        }
      }
    }
  }

  fftw_destroy_plan(p);
  
  return 0;
}
