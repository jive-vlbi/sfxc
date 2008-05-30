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

bool Fringe_info::operator==(const Fringe_info &other) const {
  assert(initialised);
  assert(other.initialised);
  return (header == other.header);
}

bool Fringe_info::operator<(const Fringe_info &other) const {
  assert(initialised);
  assert(other.initialised);
  return (header < other.header);
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
    if (std::abs(data_lag[i]) > std::abs(data_lag[index_max]))
      index_max = i;
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
  plots.clear();

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
        const size_t size = data_lag.size();
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

    int freq     = first_plot.header.frequency_nr;
    int sideband = first_plot.header.sideband;
    int pol1 = first_plot.header.polarisation1;
    int pol2 = first_plot.header.polarisation2;

    { // Compute nAutos and nCrosses
      for (iterator it = plots.begin(); it != plots.end(); it++) {
        if ((it->header.frequency_nr == freq) &&
            (it->header.sideband == sideband) &&
            (it->header.polarisation1 == pol1) &&
            (it->header.polarisation2 == pol2)) {
          if (it->header.station_nr1 == it->header.station_nr2) {
            autos.push_back(it->header.station_nr1);
          } else {
            crosses.push_back(std::make_pair(it->header.station_nr1,
                                             it->header.station_nr2));
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
        for (iterator it = plots.begin(); it != plots.end(); it++) {
          if ((it->header.frequency_nr == freq) &&
              (it->header.sideband == sideband) &&
              (it->header.polarisation1 == pol1) &&
              (it->header.polarisation2 == pol2)) {
            if (it->header.station_nr1 == it->header.station_nr2) {
              assert(it->header.station_nr1 < stations.size());
              index_html << "<th>" << stations[it->header.station_nr1] << "</th>";
            }
          }
        }
        // crosses
        for (iterator it = plots.begin(); it != plots.end(); it++) {
          if ((it->header.frequency_nr == freq) &&
              (it->header.sideband == sideband) &&
              (it->header.polarisation1 == pol1) &&
              (it->header.polarisation2 == pol2)) {
            if (it->header.station_nr1 != it->header.station_nr2) {
              assert(it->header.station_nr1 < stations.size());
              assert(it->header.station_nr2 < stations.size());
              index_html << "<th>"
              << stations[it->header.station_nr1] << "-"
              << stations[it->header.station_nr2] << "</th>";
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

        const Fringe_info &first_plot = get_first_plot();
        int curr_freq     = first_plot.header.frequency_nr;
        int curr_sideband = first_plot.header.sideband;
        int curr_pol1 = first_plot.header.polarisation1;
        int curr_pol2 = first_plot.header.polarisation2;

        begin_data_row(index_html,
                       frequencies,
                       first_plot);

        size_t column = 0;
        for (iterator it = plots.begin(); it != plots.end(); it++) {
          if (!((it->header.frequency_nr == curr_freq) &&
                (it->header.sideband == curr_sideband) &&
                (it->header.polarisation1 == curr_pol1) &&
                (it->header.polarisation2 == curr_pol2))) {
            curr_freq = it->header.frequency_nr;
            curr_sideband = it->header.sideband;
            curr_pol1 = it->header.polarisation1;
            curr_pol2 = it->header.polarisation2;
            column = 0;

            end_data_row(index_html);
            begin_data_row(index_html,
                           frequencies,
                           *it);
          }

          // Print one plot
          if (it->header.station_nr1 == it->header.station_nr2) {
            print_auto(index_html, *it);
          } else {
            while (column < autos.size()) {
              index_html << "<td></td>";
              column ++;
            }
            print_cross(index_html, *it);
          }
          column ++;
          index_html << "\n  ";
        }

      }
      end_data_row(index_html);
      index_html << "</table>" << std::endl;
    }
    index_html << "</html>" << std::endl;

    index_html.close();

    // Atomic update
    rename("index2.html", "index.html");
  }
}

void Fringe_info_container::
begin_data_row(std::ostream &index_html,
               const std::vector<double> &frequencies,
               const Fringe_info &fringe_info) {
  index_html << "<tr>" << std::endl;
  // First cell
  index_html << "<th>";

  index_html.precision(10);
  index_html << frequencies[fringe_info.header.frequency_nr]/1000000
  << "MHz";
  index_html.precision(4);
  if (fringe_info.header.sideband == 0) {
    index_html << ", LSB";
  } else {
    index_html << ", USB";
  }
  if (fringe_info.header.polarisation1 == 0) {
    index_html << ", Rcp";
  } else {
    index_html << ", Lcp";
  }
  if (fringe_info.header.polarisation2 == 0) {
    index_html << "-Rcp";
  } else {
    index_html << "-Lcp";
  }
  index_html << "</th>" << std::endl;

}

void Fringe_info_container::end_data_row(std::ostream &index_html) {
  index_html << "</tr>" << std::endl;
}

void
Fringe_info_container::set_plot(const Fringe_info &fringe_info) {
  assert(fringe_info.initialised);

  assert(plots.find(fringe_info) == plots.end());

  plots.insert(fringe_info);
}

const Fringe_info &
Fringe_info_container::
get_first_plot() const {
  assert(!plots.empty());
  return *plots.begin();
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

  if (plots.find(data) == plots.end()) {
    DEBUG_MSG("Not found: " << data.header);
    DEBUG_MSG("size " << plots.size());
    DEBUG_MSG("first      " << plots.begin()->header);
    assert(plots.find(data) != plots.end());
  }
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
           const Fringe_info &fringe_info) {
  assert(fringe_info.initialised);

  index_html << "<td>";

  char filename[80], filename_large[80], title[80];
  generate_filename(filename, filename_large, title, 80, fringe_info);
  fringe_info.plot(filename, filename_large, title, Fringe_info::FREQUENCY);
  index_html << "<A href = '" << filename_large << "' "
  << "OnMouseOver=\"show('" << filename << "');\">"
  << "A" << "</a>";

  index_html << "</td>";
}
void
Fringe_info_container::
print_cross(std::ostream &index_html,
            const Fringe_info &fringe_info) {

  if (fringe_info.initialised) {
    char filename[80], filename_large[80], title[80];
    generate_filename(filename, filename_large, title, 80, fringe_info);
    fringe_info.plot(filename, filename_large, title,
                     Fringe_info::LAG);

    double snr = fringe_info.signal_to_noise_ratio();
    int color_val =
      (int)(255*(snr-MIN_SNR_VALUE) / (MAX_SNR_VALUE-MIN_SNR_VALUE));
    if (color_val < 0)
      color_val = 0;
    if (color_val > 255)
      color_val = 255;
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
  } else {
    index_html << "<td></td>";
  }
}

void
Fringe_info_container::
print_diff(std::ostream &index_html,
           Fringe_info fringe_info1,
           const Fringe_info &fringe_info2,
           bool relative_error,
           Fringe_info::SPACE space) {

  assert(fringe_info1.initialised);
  assert(fringe_info2.initialised);
  assert(fringe_info1.header == fringe_info2.header);
  assert(fringe_info1.data_freq.size() == fringe_info2.data_freq.size());
  
  float max_diff = 0;

  index_html << "<td>";

  if (space == Fringe_info::FREQUENCY) {
    std::vector< std::complex<float> > &data1 = fringe_info1.data_freq;
    const std::vector< std::complex<float> > &data2 = fringe_info2.data_freq;
    for (size_t i=0; i<data1.size(); i++) {
      if (relative_error) {
        if (data1[i] != std::complex<float>(0.))
          data1[i] = (data1[i]-data2[i])/data1[i];
      } else {
        data1[i] -= data2[i];
      }
      max_diff = std::max(max_diff, std::abs(data1[i]));
    }
  } else {
    std::vector< std::complex<float> > &data1 = fringe_info1.data_lag;
    const std::vector< std::complex<float> > &data2 = fringe_info2.data_lag;
    for (size_t i=0; i<data1.size(); i++) {
      if (relative_error) {
        if (data1[i] != std::complex<float>(0.))
          data1[i] = (data1[i]-data2[i])/data1[i];
      } else {
        data1[i] -= data2[i];
      }
      max_diff = std::max(max_diff, std::abs(data1[i]));
    }
  }

  char filename[80], filename_large[80], title[80];
  generate_filename(filename, filename_large, title, 80, fringe_info1);
  fringe_info1.plot(filename, filename_large, title, space);
  index_html << "<A href = '" << filename_large << "' "
  << "OnMouseOver=\"show('" << filename << "');\">"
  << max_diff << "</a>";

  index_html << "</td>";
}

const Fringe_info &
Fringe_info_container::get_plot(const Output_header_baseline &h)  const {
  Fringe_info info(h,
                   std::vector< std::complex<float> >(),
                   std::vector< std::complex<float> >());
  iterator it = plots.find(info);

  if (it == plots.end())
    return empty_fringe_info;

  return *it;
}


void
Fringe_info_container::
print_diff_html(const Vex &vex,
                const Fringe_info_container &other_info,
                bool relative_error) {
  { // First check that info1 and info2 contain the same baselines
    iterator it1 = plots.begin();
    iterator it2 = other_info.plots.begin();
    while ((it1!=plots.end()) && (it2!=other_info.plots.end())) {
      if (!((*it1).header == (*it2).header)) {
        std::cout << "Error: different plots in the output files" << std::endl;
        return;
      }

      it1++;
      it2++;
    }
    if ((it1!=plots.end()) || (it2!=other_info.plots.end())) {
      std::cout << "Error: more plots in one file than the other" << std::endl;
      return;
    }
  }

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
  << "  <title>Difference plot - "<< global_header.experiment
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

    int freq     = first_plot.header.frequency_nr;
    int sideband = first_plot.header.sideband;
    int pol1 = first_plot.header.polarisation1;
    int pol2 = first_plot.header.polarisation2;

    { // Compute nAutos and nCrosses
      for (iterator it = plots.begin(); it != plots.end(); it++) {
        if ((it->header.frequency_nr == freq) &&
            (it->header.sideband == sideband) &&
            (it->header.polarisation1 == pol1) &&
            (it->header.polarisation2 == pol2)) {
          if (it->header.station_nr1 == it->header.station_nr2) {
            autos.push_back(it->header.station_nr1);
          } else {
            crosses.push_back(std::make_pair(it->header.station_nr1,
                                             it->header.station_nr2));
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
        for (iterator it = plots.begin(); it != plots.end(); it++) {
          if ((it->header.frequency_nr == freq) &&
              (it->header.sideband == sideband) &&
              (it->header.polarisation1 == pol1) &&
              (it->header.polarisation2 == pol2)) {
            if (it->header.station_nr1 == it->header.station_nr2) {
              assert(it->header.station_nr1 < stations.size());
              index_html << "<th>" << stations[it->header.station_nr1] << "</th>";
            }
          }
        }
        // crosses
        for (iterator it = plots.begin(); it != plots.end(); it++) {
          if ((it->header.frequency_nr == freq) &&
              (it->header.sideband == sideband) &&
              (it->header.polarisation1 == pol1) &&
              (it->header.polarisation2 == pol2)) {
            if (it->header.station_nr1 != it->header.station_nr2) {
              assert(it->header.station_nr1 < stations.size());
              assert(it->header.station_nr2 < stations.size());
              index_html << "<th>"
              << stations[it->header.station_nr1] << "-"
              << stations[it->header.station_nr2] << "</th>";
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

        const Fringe_info &first_plot = get_first_plot();
        int curr_freq     = first_plot.header.frequency_nr;
        int curr_sideband = first_plot.header.sideband;
        int curr_pol1 = first_plot.header.polarisation1;
        int curr_pol2 = first_plot.header.polarisation2;

        begin_data_row(index_html,
                       frequencies,
                       first_plot);

        size_t column = 0;
        iterator it2 = other_info.plots.begin();

        for (iterator it = plots.begin(); it != plots.end(); it++, it2++) {
          assert(it->header == it2->header);

          if (!((it->header.frequency_nr == curr_freq) &&
                (it->header.sideband == curr_sideband) &&
                (it->header.polarisation1 == curr_pol1) &&
                (it->header.polarisation2 == curr_pol2))) {
            curr_freq = it->header.frequency_nr;
            curr_sideband = it->header.sideband;
            curr_pol1 = it->header.polarisation1;
            curr_pol2 = it->header.polarisation2;
            column = 0;

            end_data_row(index_html);
            begin_data_row(index_html,
                           frequencies,
                           *it);
          }

          // Print one plot
          if (it->header.station_nr1 == it->header.station_nr2) {
            print_diff(index_html, *it, *it2, 
                       relative_error, Fringe_info::FREQUENCY);
          } else {
            while (column < autos.size()) {
              index_html << "<td></td>";
              column ++;
            }
            print_diff(index_html, *it, *it2, 
                       relative_error, Fringe_info::LAG);
          }
          column ++;
          index_html << "\n  ";
        }

      }
      end_data_row(index_html);
      index_html << "</table>" << std::endl;
    }
    index_html << "</html>" << std::endl;

    index_html.close();

    // Atomic update
    rename("index2.html", "index.html");
  }
}

