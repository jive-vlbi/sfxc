/* author : N.G.H. Kruithof
 * 
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
    : header(header), data(data), initialised(true) {}

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
    const int N = data.size();
    int index_max = max_value_offset();
    index_max = (index_max+N)%N;

    //return noise rms in array, skip 10 % around maximum
    std::complex<float> mean(0,0);
    int n2avg=0;

    for (size_t i=0 ; i< N ; i++){
      // difference in the range [0,n)
      int pos_diff = (N+index_max-i)%N;
      // difference in the range [-n/2,n/2)
      pos_diff = std::abs((int)((pos_diff+N/2)%N - 
                                N/2));
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
      int pos_diff = (N+index_max-i)%N;
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
  All_plots_data(const Vex &vex) : integration_slice(-1) {
    const Vex::Node root_node = vex.get_root_node();
    for (Vex::Node::const_iterator it = root_node["STATION"]->begin();
         it != root_node["STATION"]->end(); it++) {
      stations.push_back(it.key());
    }
    
    vex.get_frequencies(frequencies);
  }

  void read_plots(std::istream &in);

  void print_html();

private:
  void set_plot(const Plot_data &plot_data);

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

  // plot[sideband][polarisation1][polarisation2][Channel][station1][station2]
  Container plots[2][2][2];

  // The global header in the data
  Output_header_global global_header;

  // Integration slice for which we print the fringe
  int integration_slice;

  // Array with the station names
  std::vector<std::string> stations;

  // Array with frequencies for a channel nr
  std::vector<double>      frequencies;
};

void
All_plots_data::read_plots(std::istream &input) {
  //read-in the global header 
  input.read((char*)&global_header, sizeof(Output_header_global));
  if (input.eof()) {
    std::cout << "Empty correlation file" << std::endl;
    exit(-1);
  }

  integration_slice = -1;

  std::vector< std::complex<float> > data;
  std::vector< float >               data_float;
  data.resize(global_header.number_channels+1);
  data_float.resize(global_header.number_channels+1);
  fftwf_plan fftwf_plan_ = 
    fftwf_plan_dft_1d(global_header.number_channels+1, 
                      reinterpret_cast<fftwf_complex*>(&data[0]),
                      reinterpret_cast<fftwf_complex*>(&data[0]),
                      FFTW_BACKWARD, 
                      FFTW_ESTIMATE);
  
  while (!input.eof()) {
    
    // Read the header of the timeslice
    Output_header_timeslice timeslice_header;
    input.read((char*)&timeslice_header, sizeof(Output_header_timeslice));

    int n_baselines = timeslice_header.number_baselines;

    // Initialise the integration slice
    if (integration_slice == -1) 
      integration_slice = timeslice_header.integration_slice;

    if (!input.eof()) {
      for (int i=0; i<n_baselines; i++) {
        // Read the header of the baseline
        Output_header_baseline baseline_header;
        input.read((char*)&baseline_header, sizeof(Output_header_baseline));

        // Read the data
        input.read((char *)&data[0], 
                   data.size()*sizeof(std::complex<float>));

        if (integration_slice == timeslice_header.integration_slice) {
          if (baseline_header.station_nr1 != baseline_header.station_nr2) {
            fftwf_execute(fftwf_plan_);
            int data_size = data.size();
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
      }
    }
  }
}

void
All_plots_data::print_html() {
  std::ofstream index_html("index.html");
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
    int sideband=-1, pol1=-1, pol2=-1;
    { // Compute nAutos and nCrosses
      for (int sideband_it = 0; sideband_it<2; sideband_it++) {
        for (int pol1_it = 0; pol1_it<2; pol1_it++) {
          for (int pol2_it = 0; pol2_it<2; pol2_it++) {
            if (!plots[sideband_it][pol1_it][pol2_it].empty()) {
              if (!plots[sideband_it][pol1_it][pol2_it][0].empty()) {
                sideband = sideband_it;
                pol1 = pol1_it;
                pol2 = pol2_it;
              }
            }
          }
        }
      }
      assert(sideband != -1);
      assert(!plots[sideband][pol1][pol2].empty());

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
      generate_filename(filename, filename_large, title, 80,
                        plots[sideband][pol1][pol2][0][0][0]);
      index_html << "<td rowspan=99><img src=\"" 
                 << filename << "\" name=\"plot_image\"></td>" << std::endl;
      index_html << "</tr>" << std::endl;
    }

    { // Print content of the table
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
                  if (!plots[sideband][pol1][pol2][ch][0].empty()) {
                    if (plots[sideband][pol1][pol2][ch][0][0].initialised) {
                      assert(pol1 == pol2);
                      const Output_header_baseline &header = 
                        plots[sideband][pol1][pol2][ch][0][0].header;

                      assert(header.frequency_nr < frequencies.size());
                      index_html.precision(10);
                      index_html << frequencies[header.frequency_nr]/1000000
                                 << "MHz";
                      index_html.precision(4);
                      if (header.sideband == 0) {
                        index_html << ", LSB";
                      } else {
                        index_html << ", USB";
                      }
                      if (header.polarisation1 == 0) {
                        index_html << ", Rcp";
                      } else {
                        index_html << ", Lcp";
                      }
                      if (header.polarisation2 == 0) {
                        index_html << "-Rcp";
                      } else {
                        index_html << "-Lcp";
                      }
                    }
                  }
                  index_html << "</th>" << std::endl;
                
                  // Autos
                  for (int i=0; i<autos.size(); i++) {
                    print_auto(index_html, sideband, pol1, pol2, ch, autos[i]);
                  }

                  // Crosses
                  for (int i=0; i<crosses.size(); i++) {
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

}

void
All_plots_data::set_plot(const Plot_data &plot_data) {
  assert(plot_data.initialised);

  int freq     = plot_data.header.frequency_nr;
  int station1 = plot_data.header.station_nr1;
  int station2 = plot_data.header.station_nr2;
  int pol1     = plot_data.header.polarisation1;
  int pol2     = plot_data.header.polarisation2;
  int sideband = plot_data.header.sideband;

  if (station1 == station2) {
    assert(pol1==pol2);
  }
  
  Container &plot = plots[sideband][pol1][pol2];
  if (plot.size() <= freq) 
    plot.resize(freq+1);
  assert(plot.size() > freq);
  if (plot[freq].size() <= station1) 
    plot[freq].resize(station1+1);
  assert(plot[freq].size() > station1);
  if (plot[freq][station1].size() <= station2) 
    plot[freq][station1].resize(station2+1);
  assert(plot[freq][station1].size() > station2);

  assert(!plot[freq][station1][station2].initialised);
  plot[freq][station1][station2] = plot_data;
  assert(plot[freq][station1][station2].initialised);
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
  if (station < plots[sideband][pol1][pol2][ch].size()) {
    if (station < plots[sideband][pol1][pol2][ch][station].size()) {
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
  if (station1 < plots[sideband][pol1][pol2][ch].size()) {
    if (station2 < plots[sideband][pol1][pol2][ch][station1].size()) {
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
                       global_header.number_channels/2) << "</font>"
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

  if (!((argc == 3) || (argc == 4))) {
    std::cout << "usage: " << argv[0] << " <vex-file> <correlation_file> [<output_directory>]" 
              << std::endl;
    exit(1);
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

  All_plots_data all_plots(vex);


  // open the input file
  std::ifstream input(argv[2], std::ios::in | std::ios::binary);
  assert(input.is_open());
  // read the data in
  all_plots.read_plots(input);
  
  if (argc== 4) {
    // Goto the output directory
    int err = chdir(argv[3]);
    // Make sure it exists
    assert(err == 0);
  }
  
  all_plots.print_html();

  return 0;
}
