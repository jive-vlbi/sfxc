#include <fstream>
#include <iostream>
#include <assert.h>
#include <complex>

#include <json/json.h>
#include <fftw3.h>

// Number of frequency bins
int number_channels;
// FFT from the freq_buffer to the lag_buffer
fftw_plan p;
// Data buffers
std::complex<double> *freq_buffer, *lag_buffer;
// Input and output streams, set in main, used in (read|write)_baseline
std::ifstream in_stream;
std::ofstream out_offset, out_magn, out_phase;

// Name of the correlator output file
std::string input_filename;
// Number of subbands used in the experiment
int n_subbands;
// Number of baselines (autos + crosses)
int nr_baselines;

void initialise(char *ctrl_file) {
  Json::Value ctrl;        // Correlator control file
  { // parse the control file
    Json::Reader reader;
    std::ifstream in(ctrl_file);
    if (!in.is_open()) {
      std::cout << "Could not open control file" << std::endl;
      assert(false);
      exit(1);
    }
    bool ok = reader.parse(in, ctrl);
    if ( !ok ) {
      // report to the user the failure and their locations in the document.
      std::cout  << "Failed to parse control file\n"
                 << reader.getFormatedErrorMessages()
                 << std::endl;
      assert(false);
      exit(1);
    }
  }

  nr_baselines = ctrl["stations"].size();
  if (ctrl["cross_polarize"].asBool()) {
    if (ctrl["reference_station"].asString() != "") {
      // autos + crosses
      nr_baselines = 2*nr_baselines + 2*(2*nr_baselines-2);
    } else {
      nr_baselines = 2*nr_baselines*(2*nr_baselines+1)/2-nr_baselines;
    }
  } else {
    if (ctrl["reference_station"].asString() != "") {
      // autos + crosses
      nr_baselines = nr_baselines + (nr_baselines-1);
    } else {
      nr_baselines = nr_baselines*(nr_baselines+1)/2;
    }
  }

  number_channels    = ctrl["number_channels"].asInt();

  input_filename = ctrl["output_file"].asString().c_str()+7;
  n_subbands = ctrl["channels"].size();
}

void read_baseline() {
  in_stream.read((char *)freq_buffer, 
                 (number_channels+1)*sizeof(std::complex<double>));
}

void write_baseline() {
  fftw_execute(p); /* repeat as needed */
      
  int max_index=0; 
  double max_ampl=0;
  {
    double ampl;
    for (int i=0; i < (number_channels+1); i++) {
      ampl = std::norm(lag_buffer[i]);
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
  out_magn << std::abs(lag_buffer[max_index]) << " ";
  out_phase << std::arg(lag_buffer[max_index]) << " ";
}

int main(int argc, char *argv[]) {
  lag_buffer = NULL; freq_buffer = NULL;
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <ctrl-file>"
              << std::endl;
    exit(1);
  }

  initialise(argv[1]);
  
  freq_buffer = (std::complex<double>*)
    fftw_malloc((number_channels+1)*sizeof(std::complex<double>));
  lag_buffer = (std::complex<double>*)
    fftw_malloc((number_channels+1)*sizeof(std::complex<double>));

  p = fftw_plan_dft_1d(number_channels+1, 
                       reinterpret_cast<fftw_complex*>(freq_buffer),
                       reinterpret_cast<fftw_complex*>(lag_buffer),
                       FFTW_BACKWARD, 
                       FFTW_ESTIMATE);

  out_offset.open("offset.txt");
  out_magn.open("magnitude.txt");
  out_phase.open("phase.txt");

  std::cout << "Nr baselines: "<< nr_baselines << std::endl;

  for (int subband = 0; subband<n_subbands; subband++) {
    in_stream.open(input_filename.c_str());
    // read to the right subband
    for (int i=0; i<subband; i++) {
      for (int j=0; j<nr_baselines; j++) {
        read_baseline();
      }
    }

    // Write all data for the subband the input file points at
    while (!in_stream.eof()) {
      // Write data for the subband
      for (int baseline = 0; baseline < nr_baselines; baseline++) {
        // read in one fourier segment
        read_baseline();

        // check whether we are finished
        if (!in_stream.eof()) write_baseline();
      }
      out_offset << std::endl;
      out_magn << std::endl;
      out_phase << std::endl;

      // Skip all other subbands
      for (int i=0; i<subband-1; i++) {
        for (int baseline = 0; baseline < nr_baselines; baseline++) {
          // read in one fourier segment
          read_baseline();
        }
      }
    }
    out_offset << std::endl;
    out_magn << std::endl;
    out_phase << std::endl;

    in_stream.close();
  }

  fftw_destroy_plan(p);
}
