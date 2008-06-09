#include <iostream>
#include <fstream>
#include <assert.h>
#include <complex>

#include "output_header.h"

// Prints out all headers in the output correlation file to std::cout and 
// writes the data to the file "output_new.txt".
int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " <cor-file>" << std::endl;
    exit(-1);
  }
  char * infile = argv[1];

  std::ifstream in(infile, std::ios::binary);
  assert(in.is_open());
  std::ofstream out("output_new.txt");
  assert(out.is_open());

  // Read the global header
  struct Output_header_global global_header;
  in.read((char *)&global_header, sizeof(global_header));
  std::cout << global_header;

  std::complex<float> data[global_header.number_channels+1];

  while (!in.eof()) {
    // Read the timeslice header
    struct Output_header_timeslice timeslice_header;
    in.read((char *)&timeslice_header, sizeof(timeslice_header));
    if (in.eof()) return 0;
    std::cout << std::endl << timeslice_header;

    // read the baselines
    for (int i=0; i<(int)timeslice_header.number_baselines; i++) {
      struct Output_header_baseline baseline_header;
      in.read((char *)&baseline_header, sizeof(baseline_header));
      if (in.eof()) return 0;
      std::cout << baseline_header;

      in.read((char *)&data[0], sizeof(data));
      for (int i=0; i<global_header.number_channels+1; i++) {
        out << data[i].real() << " "
        << data[i].imag() << std::endl;
      }
    }
  }
}
