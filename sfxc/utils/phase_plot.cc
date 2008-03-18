#include <fstream>
#include <iostream>
#include <complex>
#include <assert.h>
#include <fftw3.h>
#include <output_header.h>

#define N_BASELINES (8 + (8*7)/2)

class Baseline {
public:
  Baseline(int station1_, int pol1_, int station2_, int pol2_) {
    if (station1_ < station2_) {
      station1 = station1_;
      station2 = station2_;
      pol1 = pol1_;
      pol2 = pol2_;
    } else if (station1_ == station2_) {
      assert(pol1_ == pol2);
      station1 = station1_;
      station2 = station2_;
      pol1 = pol1_;
      pol2 = pol2_;
    } else {
      station1 = station2_;
      station2 = station1_;
      pol1 = pol2_;
      pol2 = pol1_;
    }

  }
  int station1, station2;
  int pol1, pol2;

  bool operator<(const Baseline &other) {
    if ((station1 < other.station1) ||
        ((station1 == other.station1) &&
         ((pol1 < other.pol1) ||
          ((pol1 == other.pol1) &&
           ((station2 < other.station2) ||
            ((station2 == other.station2) &&
             (pol2 < other.pol2))))))) return true;

    return false;
  }
};

int main(int argc, char *argv[])
{
  assert(argc == 2);

  std::ifstream infile(argv[1], std::ios::in | std::ios::binary);
  assert(infile.is_open());

  Output_header_global global_header;
  infile.read((char *)&global_header, sizeof(global_header));
  
  std::cout << global_header << std::endl;

  const int N = global_header.number_channels;

  std::ofstream out("phases.txt");
  assert(out.is_open());

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
      out << std::endl;
    }
    if (infile.eof()) {
      out << std::endl;
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
        
        out << std::arg(output_buffer[0]) << " ";
      }
    }
  }

  fftwf_destroy_plan(p);
  
  return 0;
}
