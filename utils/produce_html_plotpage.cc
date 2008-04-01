/* 
 * author : N.G.H. Kruithof
 */

#define MAX_SNR_VALUE 8
#define MIN_SNR_VALUE 3

#include <iostream>
#include <complex>
#include <stdio.h>
#include <fstream>
#include <fftw3.h>
#include <math.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>
#include <complex>
#include "log_writer_cout.h"
#include "gnuplot_i.h"
#include "utils.h"

#include "output_header.h"
#include <vex/Vex++.h>

class Plot_data {
public:
  Plot_data() : initialised(false) {}

  Plot_data(const Output_header_baseline &header,
            const std::vector<float>     &data)
    : header(header), data(data), initialised(true) {
  }

  Output_header_baseline header;
  std::vector<float>     data;
  bool initialised;

  void plot(char *filename, char *filename_large, char *title) {
    char cmd[80];
    gnuplot_ctrl * g = gnuplot_init();

    gnuplot_cmd(g, "set terminal png tiny size 300,200");
    snprintf(cmd, 80, "set output \"%s\"", filename);
    gnuplot_cmd(g, cmd);
    gnuplot_setstyle(g, "lines");
    gnuplot_plot_x(g, &data[0], data.size(), title) ;

    gnuplot_cmd(g, "set terminal png large size 1024,768");
    snprintf(cmd, 80, "set output \"%s\"", filename_large);
    gnuplot_cmd(g, cmd);
    gnuplot_setstyle(g, "lines");
    gnuplot_plot_x(g, &data[0], data.size(), title) ;

    gnuplot_close(g);
  }

  float signal_to_noise_ratio() {
    const size_t N = data.size();
    int index_max = max_value_offset();
    index_max = (index_max+N)%N;

    //return noise rms in array, skip 10 % around maximum
    std::complex<float> mean(0,0);
    int n2avg=0;

    for (size_t i=0 ; i< N ; i++){
      // difference in the range [0,n)
      size_t pos_diff = (N+index_max-i)%N;
      // difference in the range [-n/2,n/2)
      pos_diff = std::abs((int)((pos_diff+N/2)%N - N/2));
      if (pos_diff > N/20) {
        // skip 10% arround lag for max which is at imax 
        n2avg++;
        mean += data[i];
      }
    }

    mean /= n2avg;

    float sum = 0;
    for (size_t i=0 ; i< N ; i++){
      // difference in the range [0,n)
      size_t pos_diff = (N+index_max-i)%N;
      // difference in the range [-n/2,n/2)
      pos_diff = std::abs((int)((pos_diff+N/2)%N - 
                                N/2));
      if (pos_diff > N/20) {
        sum += norm(data[i]-mean);
      }
    }

    return sqrt(norm(data[index_max]-mean)/(sum/n2avg));
  }

  int max_value_offset() {
    int index_max = 0;
    for (size_t i=1; i<data.size(); i++) {
      if (std::abs(data[i]) > std::abs(data[index_max])) index_max = i;
    }
    
    return index_max;
  }

};

// Container for all plots
class All_plots_data {
  typedef std::vector< std::vector< std::vector<Plot_data> > > Container;
public:
  All_plots_data(const Vex &vex, FILE *input);

  void read_plots(bool stop_at_eof);

  void print_html();

private:
  void read_data_from_file(int to_read, char * data, bool stop_at_eof);

  void set_plot(const Plot_data &plot_data);

  const Plot_data &get_first_plot();

  void generate_filename(char *filename, 
                         char *filename_large, 
                         char *title,
                         int size,
                         const Plot_data &data);
  void print_auto(std::ostream &index_html, 
                  int sideband, int pol1, int pol2, int ch, int station);
  void print_cross(std::ostream &index_html, 
                   int sideband, int pol1, int pol2, int ch, 
                   int station1, int station2);

  // input file
  FILE *input;

  // plot[sideband][polarisation1][polarisation2][Channel][station1][station2]
  Container plots[2][2][2];

  // The global header in the data
  Output_header_global global_header;

  // Header of the last timeslice read;
  Output_header_timeslice timeslice_header;
    
  // Array with the station names
  std::vector<std::string> stations;

  // Array with frequencies for a channel nr
  std::vector<double>      frequencies;

  // Arrays containing one fft
  fftwf_plan fftwf_plan_;
  std::vector< std::complex<float> > data;
  std::vector< float >               data_float;
};

All_plots_data::All_plots_data(const Vex &vex, FILE *input) : input(input) {
    const Vex::Node root_node = vex.get_root_node();
    for (Vex::Node::const_iterator it = root_node["STATION"]->begin();
         it != root_node["STATION"]->end(); it++) {
      stations.push_back(it.key());
    }
    
    vex.get_frequencies(frequencies);
    
    //`read-in the global header 
    read_data_from_file(sizeof(Output_header_global), 
                        (char *)&global_header, false);
    
    data.resize(global_header.number_channels+1);
    data_float.resize(global_header.number_channels+1);
    fftwf_plan_ = 
      fftwf_plan_dft_1d(global_header.number_channels+1, 
                        reinterpret_cast<fftwf_complex*>(&data[0]),
                        reinterpret_cast<fftwf_complex*>(&data[0]),
                        FFTW_BACKWARD, 
                        FFTW_ESTIMATE);

    // Read the first timeslice header:
    read_data_from_file(sizeof(Output_header_timeslice), 
                        (char*)&timeslice_header, false);
    assert(timeslice_header.number_baselines != 0);
  }

void 
All_plots_data::read_data_from_file(int to_read, char * data, 
                                    bool stop_at_eof) {
  while (!(stop_at_eof && feof(input))) {
    int read = fread(data, to_read, 1, input);
    if (read == 1) {
      return;
    } else if (read == 0) {
      if (ferror(input)) {
        exit(-1);
      }
      sleep(1);
    }
  }
}

void
All_plots_data::read_plots(bool stop_at_eof) {
  // Clear previous plots
  for (int i1 = 0; i1<2; i1++) {
    for (int i2 = 0; i2<2; i2++) {
      for (int i3 = 0; i3<2; i3++) {
        for (size_t j1 = 0; j1<plots[i1][i2][i3].size(); j1++) {
          for (size_t j2 = 0; j2<plots[i1][i2][i3][j1].size(); j2++) {
            for (size_t j3 = 0; j3<plots[i1][i2][i3][j1][j2].size(); j3++) {
              plots[i1][i2][i3][j1][j2][j3].initialised = false;
            }
          }
        }
      }
    }
  }

  int  current_integration = timeslice_header.integration_slice;
    
  bool first_timeslice_header = true;
  while (current_integration == timeslice_header.integration_slice) {
    int n_baselines = timeslice_header.number_baselines;

    for (int i=0; i<n_baselines; i++) {
      // Read the header of the baseline
      Output_header_baseline baseline_header;
      read_data_from_file(sizeof(Output_header_baseline),
                          (char*)&baseline_header, 
                          stop_at_eof && (!first_timeslice_header));
      if (baseline_header.weight == -1) {
        return;
      }
      
      // Read the data
      read_data_from_file(data.size()*sizeof(std::complex<float>), 
                          (char *)&data[0], 
                          stop_at_eof && (!first_timeslice_header));

      
      if (baseline_header.station_nr1 != baseline_header.station_nr2) {
        fftwf_execute(fftwf_plan_);
        size_t data_size = data.size();
        for (size_t j=0; j<data_size; j++) {
          data_float[j] = std::abs(data[(j+data_size/2)%data_size]);
        }
      } else {
        for (size_t j=0; j<data.size(); j++) {
          data_float[j] = std::abs(data[j]);
        }
      }
      set_plot(Plot_data(baseline_header, data_float));
    }

    { // Read the next timeslice header
      read_data_from_file(sizeof(Output_header_timeslice), 
                          (char*)&timeslice_header, stop_at_eof);
      if (timeslice_header.number_baselines == 0) {
        return;
      }
      first_timeslice_header = false;
    }
  }
}

void
All_plots_data::print_html() {
  std::ofstream index_html("index2.html");
  assert(index_html.is_open());
  index_html.precision(4);

  index_html << "<html><head>"  << std::endl
             << "  <title>SFXC output - "<< global_header.experiment 
             << "</title>" << std::endl
             << "  <style> BODY,TH,TD{font-size: 10pt }</style>" << std::endl
             << "</head>"
             <<"<body>" 
             << std::endl;
  index_html << "<script language=\"JavaScript\"><!--" << std::endl
             << "function show(imageSrc) {" << std::endl
             << "  if (document.images) document.images['plot_image'].src"
             << " = imageSrc;" << std::endl
             << "}" << std::endl
             << "//--></script>" << std::endl
             << std::endl;

  { // Print the table
    index_html << "<table border=1 bgcolor='#dddddd' cellspacing=0>"
               << std::endl;



    // Print header
    std::vector<int>                  autos;
    std::vector< std::pair<int,int> > crosses;
    const Plot_data &first_plot = get_first_plot();

    int sideband = first_plot.header.sideband;
    int pol1 = first_plot.header.polarisation1;
    int pol2 = first_plot.header.polarisation2;

    { // Compute nAutos and nCrosses
      for (size_t st1=0; st1 != plots[sideband][pol1][pol2][0].size(); st1++) {
        for (size_t st2=0; st2 != plots[sideband][pol1][pol2][0][st1].size(); st2++) {
          if (plots[sideband][pol1][pol2][0][st1][st2].initialised) {
            if (st1 == st2) {
              autos.push_back(st1);
            } else {
              crosses.push_back(std::make_pair(st1,st2));
            }
          }
        }
      }
    }

    { // first row
      index_html << "<tr>" << std::endl;
      index_html << "  <th rowspan=2>" << global_header.experiment << "</th>" 
                 << std::endl;
      index_html << "  <th colspan="<< autos.size() << ">Auto correlations</th>"
                 << std::endl;
      index_html << "  <th colspan="<< crosses.size() << ">Cross correlations</th>"
                 << std::endl;
      index_html << "</tr>" << std::endl;
    }

    { // second row
      index_html << "<tr>" << std::endl;
      // autos
      for (size_t st1=0; st1 != plots[sideband][pol1][pol2][0].size(); st1++) {
        if (st1 < plots[sideband][pol1][pol2][0][st1].size()) {
          if (plots[sideband][pol1][pol2][0][st1][st1].initialised) {
            assert(st1 < stations.size());
            index_html << "<th>" << stations[st1] << "</th>";
          }
        }
      }
      // crosses
      for (size_t st1=0; st1 != plots[sideband][pol1][pol2][0].size(); st1++) {
        for (size_t st2=0; st2 != plots[sideband][pol1][pol2][0][st1].size(); st2++) {
          if (st1 != st2) {
            if (plots[sideband][pol1][pol2][0][st1][st2].initialised) {
              assert(st1 < stations.size());
              index_html << "<th>" 
                         << stations[st1] << "-" 
                         << stations[st2] << "</th>";
            }
          }
        }
      }

      char filename[80], filename_large[80], title[80];
      generate_filename(filename, filename_large, title, 80, first_plot);
      index_html << "<td rowspan=99><img src=\"" 
                 << filename << "\" name=\"plot_image\"></td>" << std::endl;
      index_html << "</tr>" << std::endl;
    }

    { // Print content of the table
      assert(first_plot.header.station_nr1 == first_plot.header.station_nr2);
      for (size_t ch = 0; ch != plots[sideband][pol1][pol2].size(); ch++) {
        for (int sideband=0; sideband<2; sideband++) {
          for (int pol1=0; pol1<2; pol1++) {
            for (int pol2_cnt=0; pol2_cnt<2; pol2_cnt++) {
              int pol2 = (pol1 + pol2_cnt)%2;
              if (!plots[sideband][pol1][pol2].empty()) {
                if (!plots[sideband][pol1][pol2][ch].empty()) {
                  index_html << "<tr>" << std::endl;
                  // First cell
                  index_html << "<th>";

                  index_html.precision(10);
                  index_html << frequencies[ch]/1000000
                             << "MHz";
                  index_html.precision(4);
                  if (sideband == 0) {
                    index_html << ", LSB";
                  } else {
                    index_html << ", USB";
                  }
                  if (pol1 == 0) {
                    index_html << ", Rcp";
                  } else {
                    index_html << ", Lcp";
                  }
                  if (pol2 == 0) {
                    index_html << "-Rcp";
                  } else {
                    index_html << "-Lcp";
                  }
                  index_html << "</th>" << std::endl;
                
                  // Autos
                  for (size_t i=0; i<autos.size(); i++) {
                    print_auto(index_html, sideband, pol1, pol2, ch, autos[i]);
                  }

                  // Crosses
                  for (size_t i=0; i<crosses.size(); i++) {
                    print_cross(index_html, sideband, pol1, pol2, ch, 
                                crosses[i].first, crosses[i].second);
                  }
                  index_html << "</tr>" << std::endl;
                }
              }
            }
          }
        }
      }
    }
    index_html << "</table>" << std::endl;
  }
  index_html << "</html>" << std::endl;

  index_html.close();

  // Atomic update
  rename("index2.html", "index.html");
}

void
All_plots_data::set_plot(const Plot_data &plot_data) {
  assert(plot_data.initialised);

  size_t freq     = plot_data.header.frequency_nr;
  size_t station1 = plot_data.header.station_nr1;
  size_t station2 = plot_data.header.station_nr2;
  size_t pol1     = plot_data.header.polarisation1;
  size_t pol2     = plot_data.header.polarisation2;
  size_t sideband = plot_data.header.sideband;

  if (station1 == station2) {
    assert(pol1==pol2);
  }

  Container &container = plots[sideband][pol1][pol2];
  if (container.size() <= freq) 
    container.resize(freq+1);
  assert(freq < container.size());
  if (container[freq].size() <= station1) 
    container[freq].resize(station1+1);
  assert(station1 < container[freq].size());
  if (container[freq][station1].size() <= station2) 
    container[freq][station1].resize(station2+1);
  assert(station2 < container[freq][station1].size());

  assert(!container[freq][station1][station2].initialised);
  container[freq][station1][station2] = plot_data;
  assert(container[freq][station1][station2].initialised);
}

const Plot_data &
All_plots_data::
get_first_plot() {
  for (size_t channel=0; channel < 16; channel++) {
    for (int sideband=0; sideband<2; sideband++) {
      for (int pol1=0; pol1<2; pol1++) {
        for (int pol2=0; pol2<2; pol2++) {
          Container &container = plots[sideband][pol1][pol2];
          if (channel < container.size()) {
            for (size_t station=0; 
                 station<container[channel].size(); station++) {
              if (station < container[channel][station].size()) {
                if (container[channel][station][station].initialised) {
                  return container[channel][station][station];
                }
              }
            }
          }
        }
      }
    }
  }
  assert(false);
  return plots[0][0][0][0][0][0];
}

void 
All_plots_data::
generate_filename(char *filename, 
                  char *filename_large, 
                  char *title,
                  int size,
                  const Plot_data &data) {
  int sideband = data.header.sideband;
  char sideband_ch = (sideband == 0 ? 'l' : 'u');
  int channel  = data.header.frequency_nr;
  int station1 = data.header.station_nr1;
  int station2 = data.header.station_nr2;
  int pol1 = data.header.polarisation1;
  char pol1_ch = (pol1 == 0 ? 'r' : 'l');
  int pol2 = data.header.polarisation2;
  char pol2_ch = (pol2 == 0 ? 'r' : 'l');
  
  assert(plots[sideband][pol1][pol2][channel][station1][station2].initialised);
  snprintf(filename, size,
           "st%02d_%ccp-st%02d_%ccp-ch%01d-%csb.png", 
           station1, pol1_ch, station2, pol2_ch, channel, sideband_ch);
  snprintf(filename_large, size,
           "st%02d_%ccp-st%02d_%ccp-ch%01d-%csb_large.png", 
           station1, pol1_ch, station2, pol2_ch, channel, sideband_ch);
  snprintf(title, size,
           "(st%02d,%ccp)-(st%02d,%ccp) ch%01d %csb", 
           station1, pol1_ch, station2, pol2_ch, channel, sideband_ch);
}

void 
All_plots_data::
print_auto(std::ostream &index_html,
           int sideband, int pol1, int pol2, int ch, int station) {
  index_html << "<td>";
  if (station < (int)plots[sideband][pol1][pol2][ch].size()) {
    if (station < (int)plots[sideband][pol1][pol2][ch][station].size()) {
      Plot_data &plot_data = plots[sideband][pol1][pol2][ch][station][station];
      if (plot_data.initialised) {
        char filename[80], filename_large[80], title[80];
        generate_filename(filename, filename_large, title, 80, plot_data);
        plot_data.plot(filename, filename_large, title);
        index_html << "<A href = '" << filename_large << "' "
                   << "OnMouseOver=\"show('" << filename << "');\">" 
                   << "A" << "</a>";
      }
    }
  }
  index_html << "</td>";
}
void 
All_plots_data::
print_cross(std::ostream &index_html, 
            int sideband, int pol1, int pol2, int ch, 
            int station1, int station2) {
  bool show_plot = false;
  if (station1 < (int)plots[sideband][pol1][pol2][ch].size()) {
    if (station2 < (int)plots[sideband][pol1][pol2][ch][station1].size()) {
      Plot_data &plot_data = plots[sideband][pol1][pol2][ch][station1][station2];
      if (plot_data.initialised) {
        show_plot = true;
        char filename[80], filename_large[80], title[80];
        generate_filename(filename, filename_large, title, 80, plot_data);
        plot_data.plot(filename, filename_large, title);

        double snr = plot_data.signal_to_noise_ratio();
        int color_val = 
          (int)(255*(snr-MIN_SNR_VALUE) / (MAX_SNR_VALUE-MIN_SNR_VALUE));
        if (color_val < 0) color_val = 0;
        if (color_val > 255) color_val = 255;
        char color[7];
        if (color_val >= 128) {
          snprintf(color, 7, "#00%2X00", color_val);
        } else {
          snprintf(color, 7, "#%2X0000", 255-color_val);
        }
        index_html << "<td bgcolor='" << color << "'>";
        index_html << "<A href = '" << filename_large << "' "
                   << "OnMouseOver=\"show('" << filename << "');\">" 
                   << snr << "<br>"
                   << "<font size=-2>offset: " 
                   << (plot_data.max_value_offset() - 
                       global_header.number_channels/2 - 1)
                   << "</font>"
                   << "</a>";
        index_html << "</td>";
      }
    }
  }
  if (!show_plot) index_html << "<td></td>";
}



int main(int argc, char *argv[])
{
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  if ((argc < 3) || (argc > 5)) {
    std::cout << "usage: " << argv[0] << " [-f] <vex-file> <correlation_file> [<output_directory>]" 
              << std::endl;
    exit(1);
  }

  // test for -f
  bool update = false;
  if (strcmp(argv[1], "-f") == 0) {
    update = true;
    argc--;
    argv++;
  } else {
    if (argc > 4) {
      std::cout << "usage: " << argv[0] << " [-f] <vex-file> <correlation_file> [<output_directory>]" 
                << std::endl;
      exit(1);
    }
  }

  // Parse the vex file
  Vex vex;
  { 
    char * vex_file = argv[1];
    std::ifstream in(vex_file);
    if (!in.is_open()) {
      std::cout << "Could not open vex file ["<<vex_file<<"]"<< std::endl;
      return false;
    }

    // parse the vex file
    if (!vex.open(vex_file)) {
      std::cout << "Could not parse vex file ["<<vex_file<<"]" << std::endl;
      return false;
    }
  }

  // open the input file
  FILE *input = fopen(argv[2], "rb");
  assert(input != NULL);

  if (argc== 4) {
    // Goto the output directory
    int err = chdir(argv[3]);
    // Make sure it exists
    if (err != 0) {
      std::cout << "Could not go to directory " << argv[3] << std::endl;
      return -1;
    }
  }
  

  // read the data in
  All_plots_data all_plots(vex, input);

  do {
    all_plots.read_plots(!update);
    
    all_plots.print_html();

    std::cout << "Produced html page" << std::endl;
  } while (update);

  return 0;
}
