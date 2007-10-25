/* author : N.G.H. Kruithof
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
#include <Log_writer_cout.h>
#include <gnuplot_i.h>
#include <utils.h>

#include <Control_parameters.h>

Control_parameters ConPrms;

class Plot_data {
public:
  std::string job_name;
  double frequency;
  char sideband;
  std::vector<std::string> autos, crosses;
  std::vector<double> snr_crosses; // Signal to noise ratio for the crosses

  void set_size_crosses(int size_cross) {
    crosses.resize(size_cross);
    snr_crosses.resize(size_cross);
  }
};


std::vector< Plot_data > plot_data_channels;
int plot_count=0;

class Plot_generator {
public:
  Plot_generator(std::ifstream &infile, const Control_parameters &ConPrms,
                 int count_channel);

private:
  // sets the data of plot_data except for the plots
  void set_plot_data(Plot_data &data, const Control_parameters &ConPrms,
                     int count_channel);
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

  double signal_to_noise_ratio(std::vector< std::complex<double> > &data);

private:
  int nLags;
  std::vector< std::complex<double> > in, out;
  std::vector< double > magnitude;
  fftw_plan visibilities2lags; 
};

Plot_generator::Plot_generator(std::ifstream &infile, 
                               const Control_parameters &ConPrms,
                               int count_channel)
{
  Log_writer_cout log_writer;

  nLags =ConPrms.number_channels()+1;
  in.resize(nLags);
  out.resize(nLags);
  magnitude.resize(nLags);

  visibilities2lags = 
    fftw_plan_dft_1d(nLags, 
                     reinterpret_cast<fftw_complex*>(&in[0]),
                     reinterpret_cast<fftw_complex*>(&out[0]),
                     FFTW_BACKWARD, 
                     FFTW_ESTIMATE);

  std::string ref_station = ConPrms.reference_station();
  int nStations = ConPrms.number_stations();
  int cross_channel = -1;
  if (ConPrms.cross_polarize()) {
    cross_channel = ConPrms.cross_polarisation(count_channel);
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
                        ConPrms.station(0)) + ", " +
      ConPrms.sideband(ConPrms.channel(count_channel), 
                       ConPrms.station(0))+"SB, "+
      ConPrms.polarisation(ConPrms.channel(count_channel),
                           ConPrms.station(0)) + "cp ";

  } else {
    generate_auto_plots(infile, 0, nStations, plot_data[0], ConPrms);
    generate_auto_plots(infile, 0, nStations, plot_data[2], ConPrms);

    plot_data[0].job_name = 
      ConPrms.channel(count_channel)+ ", " +
      ConPrms.frequency(ConPrms.channel(count_channel), 
                        ConPrms.station(0)) + ", " +
      ConPrms.sideband(ConPrms.channel(count_channel), 
                       ConPrms.station(0))+"SB, "+
      ConPrms.polarisation(ConPrms.channel(count_channel),
                           ConPrms.station(0)) + "cp ";
    plot_data[2].job_name = 
      ConPrms.channel(cross_channel)+ ", " +
      ConPrms.frequency(ConPrms.channel(cross_channel), 
                        ConPrms.station(0)) + ", " +
      ConPrms.sideband(ConPrms.channel(cross_channel), 
                       ConPrms.station(0))+"SB, "+
      ConPrms.polarisation(ConPrms.channel(cross_channel),
                           ConPrms.station(0)) + "cp ";

    plot_data[0].job_name += ", parallel";
    plot_data[1].job_name += "<div align=right>cross</div>";
    plot_data[2].job_name += ", parallel";
    plot_data[3].job_name += "<div align=right>cross</div>";
  }
  
  // Cross correlations

  // reference station not yet implemented
  assert(ref_station == "");

  if (cross_channel == -1) {
    // Generate the cross plots
    plot_data[0].set_size_crosses(nStations*(nStations-1)/2);
    int plot_nr=0;
    for (int i=0; i<nStations; i++) {
      for (int j=i+1; j<nStations; j++) {
        generate_cross_plot(infile, i, j, plot_data[0], plot_nr, ConPrms);
        plot_nr++;
      }    
    }
  } else {
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
  }

  if (cross_channel == -1) {
    plot_data_channels.push_back(plot_data[0]);
  } else {
    for (int i=0; i<4; i++) {
      plot_data_channels.push_back(plot_data[i]);
    }
  }  
  fftw_destroy_plan(visibilities2lags);
}

void Plot_generator::set_plot_data(Plot_data & data, 
                                   const Control_parameters &ConPrms,
                                   int count_channel) {

  for (int i=0; i<ConPrms.channels_size(); i++){
    for (int j=1; j<ConPrms.number_stations(); j++){
      if(ConPrms.polarisation(ConPrms.channel(i),ConPrms.station(j)) 
         != ConPrms.polarisation(ConPrms.channel(i),ConPrms.station(0))){
        std::cout << "error in polarisation values" << std::endl;
      } else if (ConPrms.frequency(ConPrms.channel(i),ConPrms.station(j)) 
                 != ConPrms.frequency(ConPrms.channel(i),ConPrms.station(0))){
        std::cout << "error in frequency values" << std::endl;
      } else if (ConPrms.sideband(ConPrms.channel(i),ConPrms.station(j)) 
                 != ConPrms.sideband(ConPrms.channel(i),ConPrms.station(0))){
        std::cout << "error in sideband values" << std::endl;
      }
    }
  }

  data.job_name = ConPrms.experiment()+"_"+ConPrms.channel(count_channel)
    + "_" + ConPrms.polarisation(ConPrms.channel(count_channel),ConPrms.station(0)) 
    + "cp_" + ConPrms.frequency(ConPrms.channel(count_channel), ConPrms.station(0))
    + "_" + ConPrms.sideband(ConPrms.channel(count_channel), ConPrms.station(0)) + "sb";
  data.frequency = 0; //not used at this moment HO
  data.sideband = 'L'; //not used at this moment (GenPrms.get_sideband()-1 ? 'L' : 'U');
}


void 
Plot_generator::generate_auto_plots(std::ifstream &infile,
                                    int stations_start,
                                    int stations_end,
                                    Plot_data &plot_data,
                                    const Control_parameters &ConPrms) {

  for (int station=stations_start; station<stations_end; station++) {
    infile.read((char *)&in[0], 2*in.size()*sizeof(double));

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

  infile.read((char *)&in[0], 2*in.size()*sizeof(double));
  fftw_execute(visibilities2lags);
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

double 
Plot_generator::signal_to_noise_ratio(std::vector< std::complex<double> > &data)
{
  int index_max = 0;
  for (size_t i=1; i<data.size(); i++) {
    if (norm(data[i]) > norm(data[index_max])) index_max = i;
  }

  //return noise rms in array, skip 10 % around maximum
  std::complex<double> mean(0,0);
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

  double sum = 0;
  for (size_t i=0 ; i< data.size() ; i++){
    if (((int)i < ll) || ((int)i > ul)) {
      sum += norm(data[i]-mean);
    }
  }

  return sqrt(norm(data[index_max]-mean)/(sum/n2avg));
}

void print_html(const Control_parameters &ConPrms) {
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
    std::string ref_station1 = ConPrms.reference_station();
    std::string ref_station2 = "";

    if (ref_station2 == "") {
      // First row
      html_output << "<tr><td>"
                  << ConPrms.experiment()
                  << "</td>"
                  << "<th colspan='" << nStations << "'>Auto correlation</th>";
      if (ref_station1 == "") {
        // Crosses
        html_output << "<th colspan='" << nStations*(nStations-1)/2 << "'>Cross correlation</th>";
      } else {
        html_output << "<th colspan='" << nStations-1 << "'>Cross correlation</th>";
      }
      if (!show_plots) {
        html_output << "<td rowspan=99><img src=\"" 
                    << plot_data_channels[0].autos[0] 
                    << "\" name=\"plot_image\"></td>" << std::endl;
      }
      html_output << "</tr>";

      // Second row
      html_output << "<tr><td></td>";
      for (int station = 0; station < nStations; station++) {
        html_output << "<th>" << ConPrms.station(station)
                    << "</th>";
      }
      if (ref_station1 == "") {
        for (int i=0; i<nStations; i++) {
          for (int j=i+1; j<nStations; j++) {
            html_output << "<th>" << ConPrms.station(i) << "-" 
                        << ConPrms.station(j) << "</th>\n";
          }    
        }
      } else {
        for (int i=0; i<nStations; i++) {
          if (ConPrms.station(i) != ref_station1) {
            html_output << "<th>" << ref_station1
                        << "-" << ConPrms.station(i) << "</th>\n";
          }    
        }
      }
      html_output << "</tr>\n";
    } else {
      // Two reference stations
      assert(ref_station1 != "");
      //      nStations /= 2;
      // First row
      html_output
        << "<tr><td></td>"
        << "<th colspan='" << nStations << "'>Auto correlation</th>"
        << "<th colspan='" << nStations-1 << "'>Cross correlation</th>";
      if (!show_plots) {
        html_output << "<td rowspan=99><img src=\"" 
                    << plot_data_channels[0].autos[0] 
                    << "\" name=\"plot_image\"></td>" << std::endl;
      }
      html_output << "</tr>\n";
      // Second row
      html_output << "<tr><td></td>";
      for (int station = 0; station < nStations; station++) {
        html_output << "<th>" << ConPrms.station(station)
                    << "</th>";
      }

      for (int i=0; i<nStations; i++) {
        if ((ConPrms.station(i) != ref_station1) && (ConPrms.station(i) != ref_station2)) {
          html_output << "<th>" << ref_station1
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
                      << data.crosses[col] << " - " << data.snr_crosses[col] << std::endl;
        } else {
          html_output << "<A href = '" << data.crosses[col] << "' "
                      << "OnMouseOver=\"show('" << data.crosses[col] << "');\">" 
                      << data.snr_crosses[col] << "</a></td>";
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
  
  for (int channel=0; channel<ConPrms.channels_size();) {
    // generate plots for the channel
    Plot_generator(infile, ConPrms, channel);

    // find the next channel
    if (ConPrms.cross_polarize()) {
      channel ++;
      int cross_channel = 
        ConPrms.cross_polarisation(channel);
      while ((channel <
              ConPrms.number_frequency_channels()) &&
             (cross_channel >= 0) && (cross_channel < channel)) {
        channel ++;
        cross_channel = 
          ConPrms.cross_polarisation(channel);
      }
    } else {
      channel++;
    }
  }

  print_html(ConPrms);

  return 0;
}
