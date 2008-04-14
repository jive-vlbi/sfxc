#include <fstream>
#include <iostream>
#include <assert.h>
#include <complex>

#include <json/json.h>
#include <fftw3.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <ctrl-file> <#baselines>"
    << std::endl;
    exit(1);
  }

  Json::Value ctrl;        // Correlator control file
  { // parse the control file
    Json::Reader reader;
    std::ifstream in(argv[1]);
    if (!in.is_open()) {
      std::cout << "Could not open control file" << std::endl;
      assert(false);
      return false;
    }
    bool ok = reader.parse(in, ctrl);
    if ( !ok ) {
      // report to the user the failure and their locations in the document.
      std::cout  << "Failed to parse control file\n"
      << reader.getFormatedErrorMessages()
      << std::endl;
      assert(false);
      return false;
    }
  }

  int nr_baselines;
  sscanf(argv[2], "%d", &nr_baselines);

  int number_channels    = ctrl["number_channels"].asInt();
  std::string input_file = ctrl["output_file"].asString();

  std::ifstream infile(input_file.c_str()+7);

  std::complex<double> in[number_channels+1], out[number_channels+1];

  fftw_plan p;
  p = fftw_plan_dft_1d(number_channels+1,
                       reinterpret_cast<fftw_complex*>(&in),
                       reinterpret_cast<fftw_complex*>(&out),
                       FFTW_BACKWARD,
                       FFTW_ESTIMATE);

  std::ofstream out_offset("offset.txt");
  std::ofstream out_magn("magnitude.txt");
  std::ofstream out_phase("phase.txt");

  {
    bool finished = false;
    int baseline = 0;
    while (!finished) {
      // read in one fourier segment
      infile.read((char *)in, 2*(number_channels+1)*sizeof(double));

      // check whether we are finished
      finished = infile.eof();

      if (!finished) {
        fftw_execute(p); /* repeat as needed */

        int max_index=0;
        double max_ampl=0;
        {
          double ampl;
          for (int i=0; i < number_channels+1; i++) {
            ampl = std::norm(out[i]);
            if (ampl > max_ampl) {
              max_index = i;
              max_ampl = ampl;
            }
          }
        }
        out_offset << (max_index <= number_channels/2 ?
                       max_index :
                       max_index - (number_channels+1))
        << " ";
        out_magn << std::abs(out[max_index]) << " ";
        out_phase << std::arg(out[max_index]) << " ";
        if (++baseline == nr_baselines) {
          out_offset << std::endl;
          out_magn << std::endl;
          out_phase << std::endl;
          baseline = 0;
        }
      }
    }
  }

  fftw_destroy_plan(p);

}
