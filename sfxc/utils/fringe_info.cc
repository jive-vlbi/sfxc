#include <fstream>
#include "fringe_info.h"

#define MAX_SNR_VALUE 8
#define MIN_SNR_VALUE 3

Fringe_info::
Fringe_info(const Output_header_baseline &header,
            const std::vector< std::complex<float> > &data_freq_,
            const std::vector< std::complex<float> > &data_lag_) 
  : header(header), data_freq(data_freq_), data_lag(data_lag_), 
    initialised(true) {
  assert(data_freq_.size() == data_lag_.size());
}

void Fringe_info::plot(char *filename, char *filename_large, char *title, 
                       SPACE space) const {
  std::vector<float> data;
  if (space == FREQUENCY) {
    data.resize(data_freq.size());
    for (size_t i=0; i<data.size(); i++) 
      data[i] = std::abs(data_freq[i]);
  } else {
    assert(space == LAG);
    size_t size = data_lag.size();
    data.resize(size);
    for (size_t i=0; i<size; i++)
      data[i] = std::abs(data_lag[i]);
  }

  char cmd[80];
  gnuplot_ctrl * g = gnuplot_init();

  // This works on huygens
  gnuplot_cmd(g, "set terminal png small size 300,200");
  // This works on das3
  gnuplot_cmd(g, "set terminal png small picsize 300 200");

  snprintf(cmd, 80, "set output \"%s\"", filename);
  gnuplot_cmd(g, cmd);
  gnuplot_setstyle(g, "lines");
  gnuplot_plot_x(g, &data[0], data.size(), title) ;

  // This works on huygens
  gnuplot_cmd(g, "set terminal png large size 1024,768");
  // This works on das3
  gnuplot_cmd(g, "set terminal png large picsize 1024 768");

  snprintf(cmd, 80, "set output \"%s\"", filename_large);
  gnuplot_cmd(g, cmd);
  gnuplot_setstyle(g, "lines");
  gnuplot_plot_x(g, &data[0], data.size(), title) ;

  gnuplot_close(g);
}

float Fringe_info::signal_to_noise_ratio() const {
  const size_t N = data_lag.size();
  int index_max = max_value_offset();
  index_max = (index_max+N)%N;

  //return noise rms in array, skip 10 % around maximum
  std::complex<float> mean(0,0);
  int n2avg=0;

  for (size_t i=0 ; i< N ; i++) {
    // difference in the range [0,n)
    size_t pos_diff = (N+index_max-i)%N;
    // difference in the range [-n/2,n/2)
    pos_diff = std::abs((int)((pos_diff+N/2)%N - N/2));
    if (pos_diff > N/20) {
      // skip 10% arround lag for max which is at imax
      n2avg++;
      mean += std::abs(data_lag[i]);
    }
  }

  mean /= n2avg;

  float sum = 0;
  for (size_t i=0 ; i< N ; i++) {
    // difference in the range [0,n)
    size_t pos_diff = (N+index_max-i)%N;
    // difference in the range [-n/2,n/2)
    pos_diff = std::abs((int)((pos_diff+N/2)%N -
                              N/2));
    if (pos_diff > N/20) {
      sum += norm(std::abs(data_lag[i])-mean);
    }
  }

  return sqrt(std::norm(std::abs(data_lag[index_max])-mean)/(sum/n2avg));
}

int Fringe_info::max_value_offset() const {
  const size_t N = data_lag.size();
  int index_max = 0;
  for (size_t i=1; i<N; i++) {
    if (std::abs(data_lag[i]) > std::abs(data_lag[index_max])) index_max = i;
  }

  return index_max;
}


// Fringe_info_container

Fringe_info_container::Fringe_info_container(FILE *input) : input(input) {
  //`read-in the global header
  read_data_from_file(sizeof(Output_header_global),
                      (char *)&global_header, false);

  data_freq.resize(global_header.number_channels+1);
  data_lag.resize(global_header.number_channels+1);
  fftwf_plan_ =
    fftwf_plan_dft_1d(global_header.number_channels+1,
                      reinterpret_cast<fftwf_complex*>(&data_freq[0]),
                      reinterpret_cast<fftwf_complex*>(&data_lag[0]),
                      FFTW_BACKWARD,
                      FFTW_ESTIMATE);

  // Read the first timeslice header:
  read_data_from_file(sizeof(Output_header_timeslice),
                      (char*)&timeslice_header, false);
  assert(timeslice_header.number_baselines != 0);
}

void
Fringe_info_container::read_data_from_file(int to_read, char * data,
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
Fringe_info_container::read_plots(bool stop_at_eof) {
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
      read_data_from_file(data_freq.size()*sizeof(std::complex<float>),
                          (char *)&data_freq[0],
                          stop_at_eof && (!first_timeslice_header));


      fftwf_execute(fftwf_plan_);

      { // Move the fringe to the center of the plot
        std::vector< std::complex<float> > tmp = data_lag;
        const int size = data_lag.size();
        for (size_t i=0; i<size; i++) {
          data_lag[i] = tmp[(i+size/2)%size];
        }
      }
      
      set_plot(Fringe_info(baseline_header, data_freq, data_lag));
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
Fringe_info_container::print_html(const Vex &vex) {
  // Array with the station names
  std::vector<std::string> stations;

  // Array with frequencies for a channel nr
  std::vector<double>      frequencies;


  const Vex::Node root_node = vex.get_root_node();
  for (Vex::Node::const_iterator it = root_node["STATION"]->begin();
       it != root_node["STATION"]->end(); it++) {
    stations.push_back(it.key());
  }

  vex.get_frequencies(frequencies);

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
    const Fringe_info &first_plot = get_first_plot();

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
Fringe_info_container::set_plot(const Fringe_info &fringe_info) {
  assert(fringe_info.initialised);

  size_t freq     = fringe_info.header.frequency_nr;
  size_t station1 = fringe_info.header.station_nr1;
  size_t station2 = fringe_info.header.station_nr2;
  size_t pol1     = fringe_info.header.polarisation1;
  size_t pol2     = fringe_info.header.polarisation2;
  size_t sideband = fringe_info.header.sideband;

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
  container[freq][station1][station2] = fringe_info;
  assert(container[freq][station1][station2].initialised);
}

const Fringe_info &
Fringe_info_container::
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
Fringe_info_container::
generate_filename(char *filename,
                  char *filename_large,
                  char *title,
                  int size,
                  const Fringe_info &data) {
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
Fringe_info_container::
print_auto(std::ostream &index_html,
           int sideband, int pol1, int pol2, int ch, int station) {
  index_html << "<td>";
  if (station < (int)plots[sideband][pol1][pol2][ch].size()) {
    if (station < (int)plots[sideband][pol1][pol2][ch][station].size()) {
      Fringe_info &fringe_info = plots[sideband][pol1][pol2][ch][station][station];
      if (fringe_info.initialised) {
        char filename[80], filename_large[80], title[80];
        generate_filename(filename, filename_large, title, 80, fringe_info);
        fringe_info.plot(filename, filename_large, title, 
                         Fringe_info::FREQUENCY);
        index_html << "<A href = '" << filename_large << "' "
                   << "OnMouseOver=\"show('" << filename << "');\">"
                   << "A" << "</a>";
      }
    }
  }
  index_html << "</td>";
}
void
Fringe_info_container::
print_cross(std::ostream &index_html,
            int sideband, int pol1, int pol2, int ch,
            int station1, int station2) {
  bool show_plot = false;
  if (station1 < (int)plots[sideband][pol1][pol2][ch].size()) {
    if (station2 < (int)plots[sideband][pol1][pol2][ch][station1].size()) {
      Fringe_info &fringe_info = plots[sideband][pol1][pol2][ch][station1][station2];
      if (fringe_info.initialised) {
        show_plot = true;
        char filename[80], filename_large[80], title[80];
        generate_filename(filename, filename_large, title, 80, fringe_info);
        fringe_info.plot(filename, filename_large, title,
                          Fringe_info::LAG);

        double snr = fringe_info.signal_to_noise_ratio();
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
                   << (fringe_info.max_value_offset() -
                       global_header.number_channels/2 - 1)
                   << "</font>"
                   << "</a>";
        index_html << "</td>";
      }
    }
  }
  if (!show_plot) index_html << "<td></td>";
}

const Fringe_info &
Fringe_info_container::get_plot(Output_header_baseline &h) {
  size_t freq = h.frequency_nr;
  size_t sb = h.sideband;
  size_t st1 = h.station_nr1;
  size_t pol1 = h.polarisation1;
  size_t st2 = h.station_nr2;
  size_t pol2 = h.polarisation2;

  if (freq >= plots[sb][pol1][pol2].size()) 
    return empty_fringe_info;

  if (st1 >= plots[sb][pol1][pol2][freq].size()) 
    return empty_fringe_info;

  if (st2 >= plots[sb][pol1][pol2][freq][st1].size()) 
    return empty_fringe_info;

  return plots[sb][pol1][pol2][freq][st1][st2];
}
