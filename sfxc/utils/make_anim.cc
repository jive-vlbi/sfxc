#include <stdlib.h>
#include <complex>
#include <fstream>
#include <json/json.h>
#include <fftw3.h>

#include "output_header.h"
#include "gnuplot_i.h"

const char *tmp_dir = "/tmp";
const int MAX_PLOT  = 25;


Json::Value ctrl;        // Correlator control file
int st1, st2;

std::vector< std::vector< float > > data;

void read_data() {
  int read;
  std::string filename = ctrl["output_file"].asString().c_str()+7;
  FILE *input = fopen(filename.c_str(), "rb");
  assert(input != NULL);

  Output_header_global global_header;
  Output_header_timeslice timeslice_header;
  Output_header_baseline baseline_header;

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
      if (((baseline_header.station_nr1 == st1) &&
           (baseline_header.station_nr2 == st2)) ||
          ((baseline_header.station_nr1 == st2) &&
           (baseline_header.station_nr2 == st1))) {
        if (data.size() < 50) {
        // do the fft
        fftwf_execute(fftwf_plan_);
        // Store the baseline
        data.push_back(std::vector<float>());
        data.back().resize(number_channels);
        for (int j=0; j<number_channels; j++) {
          data.back()[j] = std::abs(tmp_baseline[(j+number_channels/2)%number_channels]);
        }
        }
      }
      
    }
  } while (!feof(input));
}

void show_data() {
//   int start, stop;
//   {
//     int year1, year2, day1, day2, hour, min, sec;
//     sscanf(ctrl["start"].asString().c_str(), "%dy%dd%dh%dm%ds",
//            &year1, &day1, &hour, &min, &sec);
//     start = ((hour * 60 + min) * 60 + sec);
//     sscanf(ctrl["stop"].asString().c_str(), "%dy%dd%dh%dm%ds",
//            &year2, &day2, &hour, &min, &sec);
//     assert(year1 == year2);
//     stop = (day2-day1)*24*60*60 + ((hour * 60 + min) * 60 + sec);
//   }

//   int n_integrations = (int)((stop-start)/ctrl["integr_time"].asDouble());
  int n_integrations = data.size();
  int number_channels = data[0].size();
  
  char cmd[80], data_file[80], plot_file[80];
  snprintf(data_file, 80, "%s/data.cin", tmp_dir);

  for (int img_nr=1-MAX_PLOT; img_nr < n_integrations; img_nr++) {
    std::cout << img_nr << std::endl;
    snprintf(plot_file, 80, "%s/mov%03d.png", tmp_dir, img_nr+MAX_PLOT);
    { // Write data to file
      int curr_integrations = std::min(img_nr+MAX_PLOT, (int)data.size());
      std::ofstream out(data_file);

      out << 0 << std::endl << std::endl;
      // fill the first integrations time with zeros:
      for (int i=img_nr; i<0; i++) {
        out << 0 << std::endl << std::endl;
        if ((i%5) == 0) 
          out << 0 << std::endl << std::endl;
      }
      // fill first slices with fringes
      for (int i=std::max(img_nr,0); i<curr_integrations; i++) {
        if ((i%5) == 0) 
          out << 0 << std::endl << std::endl;
        for (int j=0; j<number_channels; j++) {
          out << data[i][j] << std::endl;
        }
        out << std::endl;
      }
      // fill the remaining time with zeros:
      for (int i=curr_integrations; i<img_nr+MAX_PLOT; i++) {
        out << 0 << std::endl << std::endl;
        if ((i%5) == 0) 
          out << 0 << std::endl << std::endl;
      }
    }

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
      
      snprintf(cmd, 80, "splot \"%s\" w l t \"\"", data_file);
      gnuplot_cmd(g, cmd);
      
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
  if (!data.empty()) 
    show_data();

  char cmd[80];
  snprintf(cmd, 80, "convert -delay 50 -loop 0 %s/mov*.png anim.gif", tmp_dir);
  system(cmd);
}
