#include <fstream>
#include <iostream>
#include <complex>
#include <assert.h>
#include <fftw3.h>
#include <output_header.h>

//Writes out the phase of the fringe for every timeslice
//Each row contains the phases for every baseline, in the order that they occur 
//in the correlation file
int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <cor-file>" << std::endl;
    exit(-1);
  }

  std::ifstream infile(argv[1], std::ios::in | std::ios::binary);
  assert(infile.is_open());

  Output_header_global global_header;
  infile.read((char *)&global_header, sizeof(global_header));

  std::cout << global_header << std::endl;

  const int N = global_header.number_channels;

  std::ofstream out_phase("phases.txt");
  assert(out_phase.is_open());

  std::ofstream out_index("index.txt");
  assert(out_index.is_open());

  std::complex<float> input_buffer[N+1], output_buffer[N+1];
  fftwf_plan p;
  p = fftwf_plan_dft_1d(N+1,
                        reinterpret_cast<fftwf_complex*>(&input_buffer[0]),
                        reinterpret_cast<fftwf_complex*>(&output_buffer[0]),
                        FFTW_BACKWARD,
                        FFTW_ESTIMATE);

  int current_integration = 0;

  while (!infile.eof()) {
    // Read the timeslice header
    struct Output_header_timeslice timeslice_header;
    infile.read((char *)&timeslice_header, sizeof(timeslice_header));
    if (timeslice_header.integration_slice != current_integration) {
      out_phase << std::endl;
      out_index << std::endl;
    }
    if (infile.eof()) {
      out_phase << std::endl;
      out_index << std::endl;
      return 0;
    }

    // read the baselines
    for (int i=0; i<(int)timeslice_header.number_baselines; i++) {
      struct Output_header_baseline baseline_header;
      infile.read((char *)&baseline_header, sizeof(baseline_header));
      if (infile.eof()) return 0;

      if (timeslice_header.integration_slice==0)
        std::cout << baseline_header;

      infile.read((char *)&input_buffer[0], sizeof(input_buffer));

      { // Compute the phase
        fftwf_execute(p);

        int max_index = 0;
        for (int i=0; i<N+1; i++) {
          if (std::abs(output_buffer[i]) >
              std::abs(output_buffer[max_index])) {
            max_index = i;
          }
        }

        out_phase << std::arg(output_buffer[max_index]) << " ";
        out_index << max_index << " ";
      }
    }
  }

  fftwf_destroy_plan(p);

  return 0;
}
