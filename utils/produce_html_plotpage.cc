/* author : N.G.H. Kruithof
* 
 * Note (H. Ozdemir):
 * At the moment we loop through the stations and  produce the html output
 * assuming that the baselines are output in the correct order in the .cor file.
 * Instead we should read the baseline header and decide weather it is auto or cross
 * depending on the station1 and station2 written in that baseline header.
 * In this case we have to write the html table in the corrected order.
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
#include "control_parameters.h"
#include "output_header.h"

Control_parameters ConPrms;

class Plot_data {
public:
  std::string job_name;
  // the following should be double
  double frequency;
  char sideband;
  std::vector<std::string> autos, crosses;
  std::vector<float> snr_crosses; // Signal to noise ratio for the crosses
  std::vector<float> offset; // Offset of the maximum value of a cross with the midpoint

  void set_size_crosses(int size_cross) {
    crosses.resize(size_cross);
    snr_crosses.resize(size_cross);
    offset.resize(size_cross);
  }
};


std::vector< Plot_data > plot_data_channels;
int plot_count=0;

class Plot_generator {
public:
  Plot_generator(std::ifstream &infile, const Control_parameters &ConPrms,
                 int count_channel, int32_t &start_time);

private:
  // sets the data of plot_data except for the plots
  void set_plot_data(Plot_data &data, 
                     const Control_parameters &ConPrms,
                     int count_channel,
                     int32_t &start_time);
  void generate_auto_plots(std::ifstream &in,
                           int stations_start,
                           int stations_end,
                           Plot_data &plot_data,
                           const Control_parameters &ConPrms);
  void generate_cross_plot(std::ifstream &in,
                           int station,
                           int station2,
                           Plot_data &plot_data,
                           int plot_nr,
                           const Control_parameters &ConPrms);
  void plot(char *filename, int nPts, char *title);

  float signal_to_noise_ratio(std::vector< std::complex<float> > &data);
  float max_value_offset(std::vector< std::complex<float> > &data);

private:
  int nLags;
  std::vector< std::complex<float> > in, out;
  std::vector<float> magnitude;
  fftwf_plan visibilities2lags; 
};

Plot_generator::Plot_generator(std::ifstream &infile, 
                               const Control_parameters &ConPrms,
                               int count_channel, 
                               int32_t &start_time)
{
  Log_writer_cout log_writer;

  nLags =ConPrms.number_channels()+1;
  in.resize(nLags);
  out.resize(nLags);
  magnitude.resize(nLags);

  visibilities2lags = 
    fftwf_plan_dft_1d(nLags, 
                     reinterpret_cast<fftwf_complex*>(&in[0]),
                     reinterpret_cast<fftwf_complex*>(&out[0]),
                     FFTW_BACKWARD, 
                     FFTW_ESTIMATE);

  int reference_station = -1;
  if (ConPrms.reference_station() != "") {
    for (int i=0; i<ConPrms.number_stations(); i++) {
      if (ConPrms.station(i) == ConPrms.reference_station()) {
        reference_station = i;
      }
    }
    assert(reference_station >= 0);
  }

  int nStations = ConPrms.number_stations();
  int cross_channel = -1;
  if (ConPrms.cross_polarize()) {
    cross_channel = ConPrms.cross_channel(count_channel, 
                                      ConPrms.get_mode(start_time));
    assert((cross_channel == -1) ||
           (cross_channel > count_channel));
  }


  Plot_data plot_data[4];

  // Read the auto correlations
  if (cross_channel == -1) {
    generate_auto_plots(infile, 0, nStations, plot_data[0], ConPrms);
    plot_data[0].job_name = 
      ConPrms.channel(count_channel)+ ", " +
      ConPrms.frequency(ConPrms.channel(count_channel), 
                        ConPrms.station(0),ConPrms.get_mode(start_time)) + ", " +
      ConPrms.sideband(ConPrms.channel(count_channel), 
                       ConPrms.station(0),ConPrms.get_mode(start_time))+"SB, "+
      ConPrms.polarisation(ConPrms.channel(count_channel),
                           ConPrms.station(0),ConPrms.get_mode(start_time)) + "cp ";

  } else {
    generate_auto_plots(infile, 0, nStations, plot_data[0], ConPrms);
    generate_auto_plots(infile, 0, nStations, plot_data[2], ConPrms);

    plot_data[0].job_name = 
      ConPrms.channel(count_channel)+ ", " +
      ConPrms.frequency(ConPrms.channel(count_channel), 
                        ConPrms.station(0),ConPrms.get_mode(start_time)) + ", " +
      ConPrms.sideband(ConPrms.channel(count_channel), 
                       ConPrms.station(0),ConPrms.get_mode(start_time))+"SB, "+
      ConPrms.polarisation(ConPrms.channel(count_channel),
                           ConPrms.station(0),ConPrms.get_mode(start_time)) + "cp ";
    plot_data[2].job_name = 
      ConPrms.channel(cross_channel)+ ", " +
      ConPrms.frequency(ConPrms.channel(cross_channel), 
                        ConPrms.station(0),ConPrms.get_mode(start_time)) + ", " +
      ConPrms.sideband(ConPrms.channel(cross_channel), 
                       ConPrms.station(0),ConPrms.get_mode(start_time))+"SB, "+
      ConPrms.polarisation(ConPrms.channel(cross_channel),
                           ConPrms.station(0),ConPrms.get_mode(start_time)) + "cp ";

    plot_data[0].job_name += ", parallel";
    plot_data[1].job_name += "<div align=right>cross</div>";
    plot_data[2].job_name += ", parallel";
    plot_data[3].job_name += "<div align=right>cross</div>";
  }
  
  // Cross correlations
  if (cross_channel == -1) {
    if (ConPrms.reference_station() == "") {
      // no cross channel and no reference station
      plot_data[0].set_size_crosses(nStations*(nStations-1)/2);
      int plot_nr=0;
      for (int i=0; i<nStations; i++) {
        for (int j=i+1; j<nStations; j++) {
          generate_cross_plot(infile, i, j, plot_data[0], plot_nr, ConPrms);
          plot_nr++;
        }    
      }
    } else {
      // no cross channel and a reference station
      plot_data[0].set_size_crosses(nStations-1);
      int plot_nr=0;
      for (int i=0; i<nStations; i++) {
        if (i != reference_station) {
          generate_cross_plot(infile, i, reference_station, plot_data[0], 
                              plot_nr, ConPrms);
          plot_nr++;
        }
      }
    }
  } else {
    if (ConPrms.reference_station() == "") {
      for (int i=0; i<4; i++) {
        plot_data[i].set_size_crosses(nStations*(nStations-1)/2);
      }
      // also generating the cross correlations
      for (int i=0; i<2*nStations; i++) {
        for (int j=i+1; j<2*nStations; j++) {
          if (i+nStations != j) {
            // don't generate the "auto" cross polarisations.
            int data_nr=0;
          
            int plot_nr = 
              nStations*(nStations-1)/2 - 
              (nStations-(i%nStations))*((nStations-(i%nStations))-1)/2 +
              ((j-i)%nStations)-1;
            if ((i<nStations) && (j<nStations)) {
              data_nr = 0;
            } else if ((i>=nStations) && (j>=nStations)) {
              data_nr = 2;
            } else {
              assert(i%nStations != j%nStations);
              if (i%nStations < j%nStations) {
                data_nr = 1;
              } else {
                data_nr = 3;
                int iprime = j%nStations;
                int jprime = i%nStations;
                plot_nr = 
                  nStations*(nStations-1)/2 - 
                  (nStations-(iprime%nStations))*((nStations-(iprime%nStations))-1)/2 +
                  ((jprime-iprime)%nStations)-1;
              }
            }
            generate_cross_plot(infile, i, j,
                                plot_data[data_nr], plot_nr, ConPrms);
          }
        }    
      }
    } else {
      // cross channel and a reference station
      for (int i=0; i<4; i++) {
        plot_data[i].set_size_crosses(nStations-1);
      }
      int row_map[] = {0, 1, 3, 2};
      for (int row=0; row < 4; row++) {
        int plot_nr=0;
        for (int i=0; i<nStations; i++) {
          if (i != reference_station) {
            generate_cross_plot(infile, i, reference_station, 
                                plot_data[row_map[row]], 
                                plot_nr, ConPrms);
            plot_nr++;
          }
        }
      }
    }
  }

  if (cross_channel == -1) {
    plot_data_channels.push_back(plot_data[0]);
  } else {
    for (int i=0; i<4; i++) {
      plot_data_channels.push_back(plot_data[i]);
    }
  }  
  fftwf_destroy_plan(visibilities2lags);
}

void Plot_generator::set_plot_data(Plot_data & data, 
                                   const Control_parameters &ConPrms,
                                   int count_channel,
                                   int32_t &start_time) {

  for (int i=0; i<ConPrms.channels_size(); i++){
    for (int j=1; j<ConPrms.number_stations(); j++){
      if(ConPrms.polarisation(ConPrms.channel(i),ConPrms.station(j),
                              ConPrms.get_mode(start_time)) 
         != ConPrms.polarisation(ConPrms.channel(i),ConPrms.station(0),
                              ConPrms.get_mode(start_time))){
        std::cout << "error in polarisation values" << std::endl;
      } else if (ConPrms.frequency(ConPrms.channel(i),ConPrms.station(j),
                                  ConPrms.get_mode(start_time)) 
                 != ConPrms.frequency(ConPrms.channel(i),ConPrms.station(0),
                                  ConPrms.get_mode(start_time))){
        std::cout << "error in frequency values" << std::endl;
      } else if (ConPrms.sideband(ConPrms.channel(i),ConPrms.station(j),
                                  ConPrms.get_mode(start_time)) 
                 != ConPrms.sideband(ConPrms.channel(i),ConPrms.station(0),
                                  ConPrms.get_mode(start_time))){
        std::cout << "error in sideband values" << std::endl;
      }
    }
  }

  data.job_name = ConPrms.experiment()+"_"+ConPrms.channel(count_channel)
    + "_" + ConPrms.polarisation(ConPrms.channel(count_channel),ConPrms.station(0),
                                 ConPrms.get_mode(start_time)) 
    + "cp_" + ConPrms.frequency(ConPrms.channel(count_channel), ConPrms.station(0),
                                 ConPrms.get_mode(start_time))
    + "_" + ConPrms.sideband(ConPrms.channel(count_channel), ConPrms.station(0),
                                 ConPrms.get_mode(start_time)) + "sb";
  data.frequency = 0; //not used at this moment HO
  data.sideband = 'L'; //not used at this moment (GenPrms.get_sideband()-1 ? 'L' : 'U');
}


void 
Plot_generator::generate_auto_plots(std::ifstream &infile,
                                    int stations_start,
                                    int stations_end,
                                    Plot_data &plot_data,
                                    const Control_parameters &ConPrms) {

  //read-in the header of the baselines
  Output_header_baseline baseline;
  //the following loop is also over baselines since 
  //in auto correlation the number of 
  //baselines equals to number of stations.
  for (int station=stations_start; station<stations_end; station++) {
    infile.read((char*)&baseline, sizeof(Output_header_baseline));
    
    assert((int)baseline.station_nr1 == (int)baseline.station_nr2);
    
    //read data for this baseline
    infile.read((char *)&in[0], 2*in.size()*sizeof(float));
    for  (int lag=0; lag<nLags; lag++) {
      magnitude[lag] = abs(in[lag]);
    }
    char title[80], filename[80];

    snprintf(title, 80, "Auto %s", ConPrms.station(station).c_str());
    snprintf(filename, 80, "%s_%s_%d.png", 
             ConPrms.experiment().c_str(), 
             ConPrms.station(station).c_str(), 
             plot_count);
    plot_data.autos.push_back(filename);
    plot_count++;

    plot(filename, nLags, title);    
  }
}

void 
Plot_generator::generate_cross_plot(std::ifstream &infile,
                                    int station,
                                    int station2,
                                    Plot_data &plot_data,
                                    int plot_nr,
                                    const Control_parameters &ConPrms) {
  int nStations = ConPrms.number_stations();
  Output_header_baseline baseline;
  //read-in the header of the baselines
  infile.read((char*)&baseline, sizeof(Output_header_baseline));

  //assert(station == (int)baseline.station_nr1);
  //assert(station2 == (int)baseline.station_nr2);

  //read data for this baseline
  infile.read((char *)&in[0], 2*in.size()*sizeof(float));
  fftwf_execute(visibilities2lags);

  for  (int lag=0; lag<nLags; lag++) {
    magnitude[lag] = abs(out[(lag+nLags/2)%nLags])/nLags;
  }
  char title[80], filename[80];
  int tmp1=0, tmp2=0;
  if (station>=nStations) {
    tmp1 = 1;
    station -= nStations;
  }
  if (station2>=nStations) {
    tmp2 = 1;
    station2 -= nStations;
  }
  snprintf(title, 80, "Cross %s vs. %s", 
           ConPrms.station(station).c_str(),
           ConPrms.station(station2).c_str());
  
  snprintf(filename, 80, "%s_%s%d-%s%d_%d.png", 
           ConPrms.experiment().c_str(), 
           ConPrms.station(station).c_str(),
           tmp1,
           ConPrms.station(station2).c_str(),
           tmp2,
           plot_count);
  assert(plot_nr < plot_data.crosses.size());
  plot_data.crosses[plot_nr] = filename;
  plot_data.snr_crosses[plot_nr] = signal_to_noise_ratio(out);
  plot_data.offset[plot_nr] = max_value_offset(out);
  plot_count++;
  plot(filename, nLags, title);
}

void 
Plot_generator::plot(char *filename, int nPts, char *title) {
  char cmd[80];
  gnuplot_ctrl * g = gnuplot_init();

  gnuplot_cmd(g, "set terminal png medium size 300,200");
  snprintf(cmd, 80, "set output \"%s\"", filename);
  gnuplot_cmd(g, cmd);
  gnuplot_setstyle(g, "lines");
  gnuplot_plot_x(g, &magnitude[0], nPts, title) ;
  gnuplot_close(g);
}

float
Plot_generator::max_value_offset(std::vector< std::complex<float> > &data)
{
  int index_max = 0;
  for (size_t i=1; i<data.size(); i++) {
    if (norm(data[i]) > norm(data[index_max])) index_max = i;
  }

  float maxval = 0.0;
  int maxval_loc = 0;
  for  (int lag=0; lag<data.size(); lag++) {
    magnitude[lag] = abs(data[(lag+data.size()/2)%data.size()])/data.size();
    if(magnitude[lag] > maxval){
      maxval = magnitude[lag];
      maxval_loc =lag -  (data.size()/2);
    }
  }
  
  return maxval_loc;
}
float 
Plot_generator::signal_to_noise_ratio(std::vector< std::complex<float> > &data)
{
  int index_max = 0;
  for (size_t i=1; i<data.size(); i++) {
    if (norm(data[i]) > norm(data[index_max])) index_max = i;
  }

  //return noise rms in array, skip 10 % around maximum
  std::complex<float> mean(0,0);
  int ll=index_max - data.size()/20;//5% of range to left
  int ul=index_max + data.size()/20;//5% of range to right
  int n2avg=0;


  for (size_t i=0 ; i< data.size() ; i++){
    if ( ((int)i < ll) || ((int)i > ul) ) {
      //skip 10% arround lag for max which is at imax 
      n2avg++;
      mean += data[i];
    }
  }

  mean /= n2avg;

  float sum = 0;
  for (size_t i=0 ; i< data.size() ; i++){
    if (((int)i < ll) || ((int)i > ul)) {
      sum += norm(data[i]-mean);
    }
  }

  return sqrt(norm(data[index_max]-mean)/(sum/n2avg));
}

void print_html(const Control_parameters &ConPrms) {
  std::string reference_station = ConPrms.reference_station();

  int nAutos   = ConPrms.number_stations();
  int nCrosses = ConPrms.number_stations() -1;
  if (reference_station == "") {
    nCrosses = ConPrms.number_stations()*(ConPrms.number_stations()-1)/2;
  }

  for (int show_plots = 0; show_plots <2; show_plots++) {

    Log_writer_cout logg;
    std::ofstream html_output;
    if (show_plots) {
      html_output.open("plots.html");
    } else {
      html_output.open("index.html");
    }
    html_output << "<html><head>"  << std::endl
                << "<title>SFXC output</title>" << std::endl
                << "<style> BODY,TH,TD{font-size: 10pt }</style>" << std::endl
                << "</head>"
                <<"<body>" 
                << std::endl;
    if (!show_plots) {
      html_output << "<script language=\"JavaScript\"><!--" << std::endl
                  << "function show(imageSrc) {" << std::endl
                  << "  if (document.images) document.images['plot_image'].src = imageSrc;" << std::endl
                  << "}" << std::endl
                  << "//--></script>" << std::endl
                  << std::endl;
    }
    if (show_plots) {
      html_output << "<a href='index.html'>Show table</a><br>" << std::endl;
    } else {
      html_output << "<a href='plots.html'>Show plots</a><br>" << std::endl;
    }

    html_output << "<table border=1 bgcolor='#dddddd' cellspacing=0>\n";
    int nStations = ConPrms.number_stations();

    // First row
    html_output
      << "<tr><td></td>"
      << "<th colspan='" << nAutos << "'>Auto correlation</th>"
      << "<th colspan='" << nCrosses << "'>Cross correlation</th>";

    if (!show_plots) {
      html_output << "<td rowspan=99><img src=\"" 
                  << plot_data_channels[0].autos[0] 
                  << "\" name=\"plot_image\"></td>" << std::endl;
    }
    html_output << "</tr>\n";

    // Second row
    html_output << "<tr><td></td>";
    // Print stations for the auto correlations
    for (int station = 0; station < nStations; station++) {
      html_output << "<th>" << ConPrms.station(station)
                  << "</th>";
    }
    // Print cross correlations
    if (reference_station == "") {
      for (int i=0; i<nStations; i++) {
        for (int j=i+1; j<nStations; j++) {
          html_output << "<th>" 
                      << ConPrms.station(i)
                      << "-"
                      << ConPrms.station(j) << "</th>\n";
        }    
      }
      html_output << "</tr>\n";
      
    } else {
      for (int i=0; i<nStations; i++) {
        if ((ConPrms.station(i) != reference_station)) {
          html_output << "<th>" << reference_station
                      << "-" << ConPrms.station(i) << "</th>\n";
        }    
      }
      html_output << "</tr>\n";
    }


    // Data
    size_t auto_size=0;
    for (size_t i=0; i<plot_data_channels.size(); i++) {
      Plot_data &data = plot_data_channels[i];

      html_output << "<tr>";
      html_output << "<td>" << data.job_name << " ";

      // autos
      if (data.autos.size() != 0) {
        auto_size = data.autos.size();
        for (size_t col=0; col<data.autos.size(); col++) {
          html_output << "<td>";
          if (show_plots) {
            html_output << "<img src='" << data.autos[col] << "'>";
          } else {
            html_output << "<A href = '" << data.autos[col] << "' "
                        << "OnMouseOver=\"show('" << data.autos[col] << "');\">" 
                        << "A" << "</a>";
          }
          html_output << "</td>\n";
        }
      } else {
        html_output << "<td colspan=" << auto_size << "></td>\n";
      }

      // crosses
      assert(data.crosses.size() == data.snr_crosses.size()); 
      for (size_t col=0; col<data.crosses.size(); col++) {
        int color_val = (int)(255*(data.snr_crosses[col]-MIN_SNR_VALUE) /
                              (MAX_SNR_VALUE-MIN_SNR_VALUE));
        if (color_val < 0) color_val = 0;
        if (color_val > 255) color_val = 255;
        char color[7];
        if (color_val >= 128) {
          snprintf(color, 7, "#00%2X00", color_val);
        } else {
          snprintf(color, 7, "#%2X0000", 255-color_val);
        }
        html_output << "<td bgcolor='" << color << "'>";
        html_output.precision(4);
        if (show_plots) {
          html_output << "<img src='" << data.crosses[col] << "'> " 
                      << data.crosses[col] << " - " << data.snr_crosses[col] << " - offset: " 
                      << data.offset[col] << std::endl;
        } else {
          html_output << "<A href = '" << data.crosses[col] << "' "
                      << "OnMouseOver=\"show('" << data.crosses[col] << "');\">" 
                      << data.snr_crosses[col] <<  " <br><font size='-3'>offset: " 
                      << data.offset[col] << "</font></a></td>";
        }
      }

      html_output << "</tr>\n";


      html_output << "<tr>";
    }
    html_output << "</table>\n";
    html_output << "</body></html>" << std::endl;
  }
}


//main
int main(int argc, char *argv[])
{
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  if (!((argc == 3) || (argc == 4))) {
    std::cout << "usage: " << argv[0] << " <ctrl-file> <vex-file> [<output_directory>]" 
              << std::endl;
    exit(1);
  }

  Control_parameters ConPrms;
  Log_writer_cout logg;

  ConPrms.initialise(argv[1], argv[2], logg);

  assert(strncmp(ConPrms.get_output_file().c_str(), "file://", 7) == 0);
  std::ifstream infile(ConPrms.get_output_file().c_str()+7, 
                       std::ios::in | std::ios::binary);
  assert(infile.is_open());
  
  if (argc== 4) {
    // Goto the output directory
    int err = chdir(argv[3]);
    // Make sure it exists
    assert(err == 0);
  }
  
  Output_header_global header;
  Output_header_timeslice timeslice;

  //read-in the global header 
  infile.read((char*)&header, sizeof(Output_header_global));
    
  for (int channel=0; channel<ConPrms.channels_size();) {
    //read-in the header of the time-slice
    infile.read((char*)&timeslice, sizeof(Output_header_timeslice));
    // generate plots for the channel
    Plot_generator(infile, ConPrms, channel, header.start_time);

    // find the next channel
    if (ConPrms.cross_polarize()) {
      channel ++;
      int cross_channel = 
        ConPrms.cross_channel(channel, ConPrms.get_mode(header.start_time));
      while ((channel <
              ConPrms.number_frequency_channels()) &&
             (cross_channel >= 0) && (cross_channel < channel)) {
        channel ++;
        cross_channel = 
          ConPrms.cross_channel(channel, ConPrms.get_mode(header.start_time));
      }
    } else {
      channel++;
    }
  }

  print_html(ConPrms);

  return 0;
}
