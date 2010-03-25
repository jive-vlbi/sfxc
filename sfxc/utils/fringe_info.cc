/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
            : Aard Keimpema <keimpema@jive.nl>, 2010
  $Id$
*/
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
                       SPACE space, VALUE value) const {
  std::vector<float> data;
  if (space == FREQUENCY) {
    data.resize(data_freq.size());
    switch (value) {
    case REAL: {
      for (size_t i=0; i<data.size(); i++)
        data[i] = data_freq[i].real();
      break;
    }
    case IMAG: {
      for (size_t i=0; i<data.size(); i++)
        data[i] = data_freq[i].imag();
      break;
    }
    case ABS: {
      for (size_t i=0; i<data.size(); i++)
        data[i] = std::abs(data_freq[i]);
      break;
    }
    case PHASE: {
      for (size_t i=0; i<data.size(); i++)
        data[i] = std::arg(data_freq[i]);
      break;
    }
    }
  } else {
    assert(space == LAG);
    size_t size = data_lag.size();
    data.resize(size);
    switch (value) {
    case REAL: {
      for (size_t i=0; i<data.size(); i++)
        data[i] = data_lag[i].real();
      break;
    }
    case IMAG: {
      for (size_t i=0; i<data.size(); i++)
        data[i] = data_lag[i].imag();
      break;
    }
    case ABS: {
      for (size_t i=0; i<data.size(); i++)
        data[i] = std::abs(data_lag[i]);
      break;
    }
    case PHASE: {
      for (size_t i=0; i<data.size(); i++)
        data[i] = std::arg(data_lag[i]);
      break;
    }
    }
  }

  char cmd[80];
  gnuplot_ctrl * g = gnuplot_init();

  // This works on huygens
  gnuplot_cmd(g, (char*)"set terminal png small size 300,200");
  // This works on das3
  gnuplot_cmd(g, (char*)"set terminal png small picsize 300 200");

  snprintf(cmd, 80, "set output \"%s\"", filename);
  gnuplot_cmd(g, cmd);
  gnuplot_setstyle(g, (char*)"lines");
  gnuplot_plot_x(g, &data[0], data.size(), title) ;

  // This works on huygens
  gnuplot_cmd(g, (char*)"set terminal png large size 1024,768");
  // This works on das3
  gnuplot_cmd(g, (char*)"set terminal png large picsize 1024 768");

  snprintf(cmd, 80, "set output \"%s\"", filename_large);
  gnuplot_cmd(g, cmd);
  gnuplot_setstyle(g, (char*)"lines");
  gnuplot_plot_x(g, &data[0], data.size(), title) ;

  gnuplot_close(g);
}

float Fringe_info::signal_to_noise_ratio() const {
  const size_t N = data_lag.size();
  int index_max = max_value_offset();
  index_max = (index_max+N)%N;

  if (data_lag[index_max] == std::complex<float>(0,0)) return 0;

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

Fringe_info_container::
Fringe_info_container(FILE *input, bool stop_at_eof) : input(input) {
  // read-in the global header
  read_data_from_file(sizeof(Output_header_global),
                      (char *)&global_header, stop_at_eof);
  if (eof()) return;

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
                      (char*)&last_timeslice_header, stop_at_eof);
  if (eof()) return;
  
  // Read the UVW coordinates, these are not used at the moment
  for(int i=0 ; i<last_timeslice_header.number_uvw_coordinates ; i++){
    struct Output_uvw_coordinates uvw_coordinates;
    read_data_from_file(sizeof(Output_uvw_coordinates),
                      (char*)&uvw_coordinates, stop_at_eof);
    if (eof()) return;
  }

  // Read in the bit statistics and process them
  new_statistics.resize(last_timeslice_header.number_statistics);
  read_data_from_file(sizeof(Output_header_bitstatistics)*new_statistics.size(),
                     (char*)&new_statistics[0], stop_at_eof);

  assert(last_timeslice_header.number_baselines != 0);
}

bool Fringe_info_container::eof() {
  return feof(input);
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
  first_timeslice_header = last_timeslice_header;

  bool first = true;
  while (first_timeslice_header.integration_slice ==
         last_timeslice_header.integration_slice) {
    process_new_bit_statistics();
    int n_baselines = last_timeslice_header.number_baselines;

    for (int i=0; i<n_baselines; i++) {
      // Read the header of the baseline
      Output_header_baseline baseline_header;
      read_data_from_file(sizeof(Output_header_baseline),
                          (char*)&baseline_header,
                          stop_at_eof && (!first));
      if (baseline_header.weight == -1) {
        return;
      }

      // Read the data
      read_data_from_file(data_freq.size()*sizeof(std::complex<float>),
                          (char *)&data_freq[0],
                          stop_at_eof && (!first));


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
                          (char*)&last_timeslice_header, stop_at_eof);
      if (last_timeslice_header.number_baselines == 0) {
        return;
      }
      first = false;
      // Read the UVW coordinates, these are not used at the moment
      for(int i=0 ; i<last_timeslice_header.number_uvw_coordinates ; i++){
        struct Output_uvw_coordinates uvw_coordinates;
        read_data_from_file(sizeof(Output_uvw_coordinates),
                      (char*)&uvw_coordinates, stop_at_eof);
        if (eof()) return;
      }
      // Read in the bit statistics and process them
      new_statistics.resize(last_timeslice_header.number_statistics);
      read_data_from_file(sizeof(Output_header_bitstatistics)*new_statistics.size(),
                         (char*)&new_statistics[0], stop_at_eof);
    }
  }
}

void
Fringe_info_container::process_new_bit_statistics(){
  for(int i=0;i<new_statistics.size();i++){
    Output_header_bitstatistics stats;
    stats.station_nr=new_statistics[i].station_nr; 
    stats.frequency_nr=new_statistics[i].frequency_nr;
    stats.sideband=new_statistics[i].sideband;
    stats.polarisation=new_statistics[i].polarisation;
    statistics.insert(new_statistics[i]);
  }
}


void
Fringe_info_container::get_bbc(const Vex &vex, std::vector<std::string> &stations, std::string &mode,
                               std::vector< std::vector<int> > &bbcs)
{
  Vex::Node root_node = vex.get_root_node();
  for(int station=0; station < stations.size(); station++){
    std::string freq = vex.get_frequency(mode, stations[station]);
    std::string bbc = vex.get_BBC(mode, stations[station]);
    std::vector<int> bbc_list;
    if(freq != std::string()){
      for(Vex::Node::iterator freq_it = root_node["FREQ"][freq]->begin("chan_def");
          freq_it != root_node["FREQ"][freq]->end("chan_def"); freq_it++){
        std::string bbc_name=(*freq_it)[5]->to_string();

        // Find the physical BBC number
        for(Vex::Node::iterator bbc_it = root_node["BBC"][bbc]->begin("BBC_assign");
            bbc_it != root_node["BBC"][bbc]->end("BBC_assign"); bbc_it++){
          std::string cur_bbc = (*bbc_it)[0]->to_string();
          if(cur_bbc == bbc_name){
            int bbc_index = (*bbc_it)[1]->to_int();
            bbc_list.push_back(bbc_index);
          }
        }
      }
    }
    bbcs.push_back(bbc_list);
  }
}

bool
Fringe_info_container::get_channels(const Vex &vex, const std::string &mode, std::vector<Channel> &channels)
{
  Vex::Node root_node = vex.get_root_node();
  Vex::Node::iterator mode_it = root_node["MODE"][mode];
  std::string freq_node = mode_it->begin("FREQ")[0]->to_string();
  std::string station = mode_it->begin("FREQ")[1]->to_string();
  std::string if_node = vex.get_IF(mode,station);
  std::string bbc_node = vex.get_BBC(mode,station);

  channels.resize(0);
  int freq_nr=-1;
  double old_freq=-1;
  for (Vex::Node::iterator chan_it = root_node["FREQ"][freq_node]->begin("chan_def");
       chan_it != root_node["FREQ"][freq_node]->end("chan_def"); ++chan_it) {
    Channel new_chan;
    double new_freq = (*chan_it)[1]->to_double_amount("MHz")*1000000;
    if(new_freq!=old_freq){
      old_freq=new_freq;
      freq_nr++;
    }
    new_chan.frequency_nr = freq_nr;
    new_chan.frequency = new_freq;
    new_chan.sideband=(*chan_it)[2]->to_char()=='L'?0:1;
    std::string bbc = (*chan_it)[5]->to_string();
    std::string if_name;
    for (Vex::Node::const_iterator bbc_block = vex.get_root_node()["BBC"][bbc_node]->begin();
         bbc_block != vex.get_root_node()["BBC"][bbc_node]->end(); ++bbc_block) {
      if(bbc_block[0]->to_string()==bbc){
        if_name = bbc_block[2]->to_string();
        break;
      }
    }
    char pol = vex.polarisation(if_node, if_name);
    new_chan.polarization=(pol=='R')?0:1;
    channels.push_back(new_chan);
  }
  return true;
}

void
Fringe_info_container::print_html(const Vex &vex, char *vex_filename) {
  // Array with the station names
  std::vector<std::string> stations;

  // Array with frequencies for a channel nr
  std::vector<double>      frequencies;
  std::vector< std::vector<int> >      bbcs;


  const Vex::Node root_node = vex.get_root_node();

  // Get an alphabetically sorted list of stations
  std::set<std::string> station_names;
  for (Vex::Node::const_iterator it = root_node["STATION"]->begin();
       it != root_node["STATION"]->end(); it++) {
    station_names.insert(it.key());
  }
  for (std::set<std::string>::iterator it = station_names.begin();
       it != station_names.end(); it++) {
    stations.push_back(*it);
  }

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

  index_html << "<a href='" << vex_filename << "'>Vex file</a> -- "
  << std::endl;
  double integration_time = pow(2, global_header.integration_time);
  double sec = (global_header.start_time +
                integration_time * first_timeslice_header.integration_slice);

  Date start_time(global_header.start_year, global_header.start_day, (int) sec);
  std::string mode = vex.get_mode(vex.get_scan_name(start_time));
  vex.get_frequencies(mode,frequencies);
  get_bbc(vex, stations, mode, bbcs);

  index_html << " Integration time: "
             << integration_time << "s"
             << " -- Start of the integration: "
             << Date(global_header.start_year,
                     global_header.start_day,
                     (int)sec).to_string()
             << (sec-std::floor(sec))*1000 << "ms"
             << std::endl;

  { // Print the table
    index_html << "<table border=1 bgcolor='#dddddd' cellspacing=0>" << std::endl;

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
        index_html << "  <th rowspan=2>" << global_header.experiment << "</th>" << std::endl;
        index_html << "  <th colspan="<< autos.size() << ">Auto correlations</th>" << std::endl;
        index_html << "  <th colspan="<< crosses.size() << ">Cross correlations</th>" << std::endl;
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
        generate_filename(filename, filename_large, title, 80, first_plot,
                          Fringe_info::FREQUENCY, Fringe_info::ABS);
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
        int freq_index=0;
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
            if(curr_pol1 == curr_pol2) 
              freq_index++; // don't increment when doing cross-polarizations
            end_data_row(index_html);
            begin_data_row(index_html,
                           frequencies,
                           *it);
          }
          // Print one plot
          if (it->header.station_nr1 == it->header.station_nr2) {
            int bbc = bbcs[(int)it->header.station_nr1][freq_index];
            print_auto(index_html, *it, bbc);
          } else {
            if (column < autos.size()) {
              index_html << "<td colspan='" << autos.size()-column << "'>Cross hands</td>";
              column=autos.size();
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

    // Print the bit statistics
    print_html_bitstatistics(vex, mode, index_html);
    index_html << "</html>" << std::endl;

    index_html.close();

    // Atomic update
    rename("index2.html", "index.html");
  }
}

std::string Fringe_info_container::get_statistics_color(int64_t val, int64_t N){
  // Compute color for the html_plot_page according to how many standard
  // deviations val is away from the average assuming a binomial distribution
  // where 0 and 1 are equally likely.
  if(N==0)
    return std::string("#FF0000");

  double std = sqrt(N)/2;
  double n_std = fabs((val-N/2)*1./std);
  if(n_std <= 2){
    char color[8];
    int color_val = 255-128*n_std/2;
    snprintf(color, 7, "#00%2X00", color_val);
    return std::string(color);
  }else if (n_std <= 3)
    return std::string("#FF8C00");

  return std::string("#FF0000");
}
void Fringe_info_container::
print_html_bitstatistics(const Vex &vex, const std::string &mode, std::ofstream &index_html){
  // Array with all channels sorted on channel nr
  std::vector<Channel> channels;
  get_channels(vex, mode, channels);

  // Get an alphabetically sorted list of stations
  const Vex::Node root_node = vex.get_root_node();
  std::set<std::string> station_names;
  for (Vex::Node::const_iterator it = root_node["STATION"]->begin();
       it != root_node["STATION"]->end(); it++) {
    station_names.insert(it.key());
  }
  std::vector<std::string> stations;
  for (std::set<std::string>::iterator it = station_names.begin();
       it != station_names.end(); it++) {
    stations.push_back(*it);
  }

  index_html << "<h1> Sampler statistics </h1><br>" << std::endl;
  // Iterate over all stations
  statistics_set::iterator it=statistics.begin();
  while(it!=statistics.end()){
    // Write table header
    index_html << "<table border=1 bgcolor='#dddddd' cellspacing=0>" << std::endl;
    index_html << "<tr>" << "\n";
    index_html << "  <th>" << stations[(int)(*it).station_nr] << "</th>" << std::endl;
    index_html << "  <th> - - </th>" << std::endl;
    index_html << "  <th> - + </th>"<< std::endl;
    index_html << "  <th> + - </th>" << std::endl;
    index_html << "  <th> + + </th>"<< std::endl;
    index_html << "  <th> invalid </th>"<< std::endl;
    index_html << "  <th> avg sign bit </th>"<< std::endl;
    index_html << "  <th> avg mag bit </th>"<< std::endl;
    index_html << "</tr>" << std::endl;

    // Iterate over all channels
    for(int i=0;i<channels.size();i++){
      // Find the current channel in the statistics set
      Output_header_bitstatistics chan;
      chan.station_nr=(*it).station_nr;
      chan.frequency_nr=channels[i].frequency_nr;
      chan.sideband=channels[i].sideband;
      chan.polarisation=channels[i].polarization;
      statistics_set::iterator chan_it=statistics.find(chan);
      // Get the total number of samples
      const int32_t (&levels)[4] = (*chan_it).levels;
      int32_t n_invalid = (*chan_it).n_invalid;
      int64_t N=0;
      for(int j=0;j<4;j++)
        N += levels[j];
      N+=n_invalid;
      // Print the statistics
      begin_data_row(index_html, channels[i]);
      for(int j=0;j<4;j++){
        index_html << "  <td> " << levels[j]*100./N <<"%" << "</td>";
      }
      index_html << "  <td> " << n_invalid*100./N <<"%" << "</td>";
      double b0=(levels[2]+levels[3])*1./(N-n_invalid);
      index_html << "  <td> " << b0 << "</td>";
      if((levels[0]>0)||(levels[3]>0)){
        double b1=(levels[1]+levels[3])*1./(N-n_invalid);
        index_html << "  <td> " << b1 << "</td>";
      }else
        index_html << "  <td> ----------- </td>";
/*      std::string color = get_statistics_color(levels[2]+levels[3],N-n_invalid);
      index_html << "  <td bgcolor=" << color << "> " << b0 << "</td>";
      color = get_statistics_color(levels[1]+levels[3],N-n_invalid);
      index_html << "  <td bgcolor=" << color << "> " << b1 << "</td>";
      end_data_row(index_html); */
      index_html << "\n  ";
    }
    index_html << "</table><br>" << std::endl;
    // Find the next station within the set
    int cur_station=(*it).station_nr;
    while((it!=statistics.end())&&(cur_station==(*it).station_nr))
      it++;
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

void Fringe_info_container::
begin_data_row(std::ostream &index_html, Channel &channel){
  index_html << "<tr>" << std::endl;
  // First cell
  index_html << "<th>";

  index_html.precision(10);
  index_html << channel.frequency/1000000 << "MHz";
  index_html.precision(4);
  if (channel.sideband == 0) {
    index_html << ", LSB";
  } else {
    index_html << ", USB";
  }
  if (channel.polarization == 0) {
    index_html << ", Rcp";
  } else {
    index_html << ", Lcp";
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
                  const Fringe_info &data,
                  const Fringe_info::SPACE space,
                  const Fringe_info::VALUE value) {
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
           "st%02d_%ccp-st%02d_%ccp-ch%01d-%csb-%d-%d.png",
           station1, pol1_ch, station2, pol2_ch, channel, sideband_ch,
           (int)space, (int)value);
  snprintf(filename_large, size,
           "st%02d_%ccp-st%02d_%ccp-ch%01d-%csb-%d-%d_large.png",
           station1, pol1_ch, station2, pol2_ch, channel, sideband_ch,
           (int)space, (int)value);
  snprintf(title, size,
           "(st%02d,%ccp)-(st%02d,%ccp) ch%01d %csb",
           station1, pol1_ch, station2, pol2_ch, channel, sideband_ch);
}

void
Fringe_info_container::
print_auto(std::ostream &index_html, const Fringe_info &fringe_info, int bbc) {
  assert(fringe_info.initialised);

  index_html << "<td>";

  char filename[80], filename_large[80], title[80];
  generate_filename(filename, filename_large, title, 80, fringe_info,
                    Fringe_info::FREQUENCY, Fringe_info::ABS);
  fringe_info.plot(filename, filename_large, title,
                   Fringe_info::FREQUENCY, Fringe_info::ABS);
  index_html << "<A href = '" << filename_large << "' "
  << "OnMouseOver=\"show('" << filename << "');\">"
  << bbc << "</a>";

  index_html << "</td>";
}
void
Fringe_info_container::
print_cross(std::ostream &index_html,
            const Fringe_info &fringe_info) {

  if (fringe_info.initialised) {
    char filename_abs[80], filename_large_abs[80], title[80];
    generate_filename(filename_abs, filename_large_abs,
                      title, 80, fringe_info,
                      Fringe_info::FREQUENCY, Fringe_info::ABS);
    fringe_info.plot(filename_abs, filename_large_abs, title,
                     Fringe_info::FREQUENCY, Fringe_info::ABS);

    char filename_phase[80], filename_large_phase[80];
    generate_filename(filename_phase, filename_large_phase,
                      title, 80, fringe_info,
                      Fringe_info::FREQUENCY, Fringe_info::PHASE);
    fringe_info.plot(filename_phase, filename_large_phase, title,
                     Fringe_info::FREQUENCY, Fringe_info::PHASE);

    char filename[80], filename_large[80];
    generate_filename(filename, filename_large,
                      title, 80, fringe_info,
                      Fringe_info::LAG, Fringe_info::ABS);
    fringe_info.plot(filename, filename_large, title,
                     Fringe_info::LAG, Fringe_info::ABS);

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
    if (fringe_info.header.polarisation1 ==
        fringe_info.header.polarisation2) {
    index_html << "<td bgcolor='" << color << "'>";
    } else {
    index_html << "<td>";
    }
    index_html << "<A href = '" << filename_large << "' "
    << "OnMouseOver=\"show('" << filename << "');\">"
    << snr << "</a>"
    << " <A href = '" << filename_large_abs << "' "
    << "OnMouseOver=\"show('" << filename_abs << "');\">"
    << "A" << "</a>"
    << " <A href = '" << filename_large_phase << "' "
    << "OnMouseOver=\"show('" << filename_phase << "');\">"
    << "P" << "</a>"
    << "<br>"
    << "<font size=-2>offset: "
    << (fringe_info.max_value_offset() -
        global_header.number_channels/2 - 1)
    << "</font>";
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
  generate_filename(filename, filename_large, title, 80, fringe_info1,
                    space, Fringe_info::ABS);
  fringe_info1.plot(filename, filename_large, title, space, Fringe_info::ABS);
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

  // Get the mode of the current scan
  double integration_time = pow(2, global_header.integration_time);
  double sec = (global_header.start_time + integration_time * first_timeslice_header.integration_slice);
  Date start_time(global_header.start_year, global_header.start_day, (int) sec);
  std::string mode = vex.get_mode(vex.get_scan_name(start_time));

  // Array with the station names
  std::vector<std::string> stations;

  // Array with frequencies for a channel nr
  std::vector<double>      frequencies;

  const Vex::Node root_node = vex.get_root_node();
  for (Vex::Node::const_iterator it = root_node["STATION"]->begin();
       it != root_node["STATION"]->end(); it++) {
    stations.push_back(it.key());
  }

  vex.get_frequencies(mode, frequencies);

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
        generate_filename(filename, filename_large,
                          title, 80, first_plot,
                          Fringe_info::FREQUENCY, Fringe_info::ABS);
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

