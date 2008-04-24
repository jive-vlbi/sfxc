#include <stdlib.h>
#include <complex>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <fftw3.h>

#include "output_header.h"
#include "gnuplot_i.h"

const char *tmp_dir = "/tmp";
const int MAX_PLOT  = 25;


Json::Value ctrl;        // Correlator control file
int st1, st2;

std::vector< std::string > data_files;


inline bool
operator==(const Output_header_baseline &bh1,
           const Output_header_baseline &bh2) {
  return ((bh1.station_nr1 == bh2.station_nr1) &&
          (bh1.station_nr2 == bh2.station_nr2) &&
          (bh1.polarisation1 == bh2.polarisation1) &&
          (bh1.polarisation2 == bh2.polarisation2) &&
          (bh1.sideband == bh2.sideband) &&
          (bh1.frequency_nr == bh2.frequency_nr));
}

void get_filename(char *filename) {
  int plot_nr = (int)data_files.size();
  snprintf(filename, 80, "%s/fringe_%03d.txt", tmp_dir, plot_nr);
}

void read_data() {
  char filename[80];

  int read;
  std::string in_filename = ctrl["output_file"].asString().c_str()+7;
  FILE *input = fopen(in_filename.c_str(), "rb");
  assert(input != NULL);

  Output_header_global global_header;
  Output_header_timeslice timeslice_header;
  Output_header_baseline baseline_header;
  Output_header_baseline baseline_header_cmp;
  baseline_header_cmp.station_nr1 = uint8_t(-1);
  bool found_baseline = false;

  read = fread(&global_header, sizeof(global_header), 1, input);
  if (read != 1) {
    std::cout << __LINE__ << " didn't read enough data" << std::endl;
  }

  int number_channels = global_header.number_channels+1;
  std::complex<float> tmp_baseline[number_channels];
  fftwf_plan fftwf_plan_ =
    fftwf_plan_dft_1d(number_channels,
                      reinterpret_cast<fftwf_complex*>(&tmp_baseline[0]),
                      reinterpret_cast<fftwf_complex*>(&tmp_baseline[0]),
                      FFTW_BACKWARD,
                      FFTW_ESTIMATE);


  for (int i=0; i<MAX_PLOT-1; i++) {
    get_filename(filename);
    std::ofstream out(filename);
    out << 0 << " " << i << " 0" << std::endl;
    out << number_channels << " " << i << " 0" << std::endl;
    data_files.push_back(filename);
  }


  do {
    read = fread(&timeslice_header, sizeof(timeslice_header), 1, input);
    if (read != 1) {
      timeslice_header.number_baselines = 0;
    }
    for (int i=0; i<timeslice_header.number_baselines; i++) {
      read = fread(&baseline_header, sizeof(baseline_header), 1, input);
      if (read != 1) {
        std::cout << __LINE__ << " didn't read enough data" << std::endl;
        i = timeslice_header.number_baselines;
        baseline_header.station_nr1 = uint8_t(-1);
      }
      read = fread(&tmp_baseline[0],
                   number_channels*sizeof(std::complex<float>), 1,
                   input);
      if (read != 1) {
        std::cout << __LINE__ << " didn't read enough data" << std::endl;
      }
      std::cout << (int)baseline_header.station_nr1 << " " << (int)baseline_header.station_nr2 << std::endl;
      if (!found_baseline) {
        if (((baseline_header.station_nr1 == st1) &&
             (baseline_header.station_nr2 == st2)) ||
            ((baseline_header.station_nr1 == st2) &&
             (baseline_header.station_nr2 == st1))) {
          baseline_header_cmp = baseline_header;
          found_baseline = true;
        }
      }
      if (baseline_header == baseline_header_cmp) {
        // do the fft
        fftwf_execute(fftwf_plan_);
        // Store the baseline
        get_filename(filename);
        std::ofstream out(filename);
        for (int j=0; j<number_channels; j++) {
          out << j << " " << data_files.size() << " "
          << std::abs(tmp_baseline[(j+number_channels/2)%number_channels])
          << std::endl;
        }
        data_files.push_back(filename);
      }
    }
  } while (!feof(input));

}

void show_data() {
  int n_integrations = data_files.size();
  std::cout << "Size: " << n_integrations << std::endl;

  char cmd[80], data_file[80], plot_file[80];
  snprintf(data_file, 80, "%s/data.cin", tmp_dir);

  for (int img_nr=0; img_nr < n_integrations-MAX_PLOT; img_nr++) {
    //for (int img_nr=10; img_nr < 11; img_nr++) {
    std::cout << img_nr << std::endl;
    snprintf(plot_file, 80, "%s/mov%03d.png", tmp_dir, img_nr+MAX_PLOT);
    { // Plot the data
      gnuplot_ctrl * g = gnuplot_init();

      gnuplot_setstyle(g, "lines");
      //gnuplot_cmd(g, "set hidden3d");

      gnuplot_cmd(g, "set terminal png");
      gnuplot_cmd(g, "set autoscale");

      gnuplot_cmd(g, "set noxtics");
      gnuplot_cmd(g, "set noytics");
      gnuplot_cmd(g, "set noztics");
      gnuplot_cmd(g, "set noborder");
      gnuplot_cmd(g, "set view 70,150,.75,1.5");

      snprintf(cmd, 80, "set output \"%s\"", plot_file);
      gnuplot_cmd(g, cmd);

      bool first_plot = true;
      std::stringstream plot_cmd;
      plot_cmd << "splot ";
      for (int i=0; i<MAX_PLOT; i++) {

        if ((img_nr+i >= 0) && (img_nr+i < (int)data_files.size())) {
          if (!first_plot) {
            plot_cmd << ", ";
          } else {
            first_plot = false;
          }
          plot_cmd << "\"" << data_files[i+img_nr] << "\"  w l lt " << ((i+img_nr)%4)+1 << " notitle";
        }
      }
      char plot_cmd2[plot_cmd.str().size()];
      strcpy(plot_cmd2, plot_cmd.str().c_str());
      gnuplot_cmd(g, plot_cmd2);

      gnuplot_close(g);
    }
  }
}

int main(int argc, char * argv[]) {
  if (argc != 4) {
    std::cout << "Usage: "
    << argv[0] << " <ctrl-file> <station1_nr> <station2_nr>"
    << std::endl;
    exit(-1);
  }

  char * ctrl_file = argv[1];
  st1 = atoi(argv[2]);
  st2 = atoi(argv[3]);

  { // parse the control file
    Json::Reader reader;
    std::ifstream in(ctrl_file);
    if (!in.is_open()) {
      std::cout << "Could not open control file [" << ctrl_file << "]" << std::endl;
      exit(-1);
    }
    bool ok = reader.parse(in, ctrl);
    if ( !ok ) {
      // report to the user the failure and their locations in the document.
      std::cout  << "Failed to parse control file\n"
      << reader.getFormatedErrorMessages()
      << std::endl;
      exit(-1);
    }
  }

  read_data();
  if (!data_files.empty())
    show_data();

  char cmd[80];
  snprintf(cmd, 80, "convert -delay 50 -loop 0 %s/mov*.png anim.gif", tmp_dir);
  system(cmd);
}
