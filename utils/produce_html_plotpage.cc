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

struct Plot_data {
  std::string job_name;
  double frequency;
  char sideband;
  std::vector<std::string> autos, crosses;
  std::vector<double> snr_crosses; // Signal to noise ratio for the crosses
};


std::vector< Plot_data > plot_data_channels;
int plot_nr=0;

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
  void generate_cross_plots(std::ifstream &in,
                            int nStations,
                            const std::string &ref_station,
														Plot_data &plot_data,
														const Control_parameters &ConPrms);
  void generate_cross_plot(std::ifstream &in,
                           const std::string &ref_station,
													 int station2,
													 Plot_data &plot_data,
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

/*  int err = 
    initialise_control(filename, log_writer, ConPrms);
  if (err != 0) return;
*/
  log_writer(0) << ConPrms.get_output_file() << std::endl;
  
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

//	std::ifstream infile(ConPrms.get_output_file().c_str(), std::ios::in | std::ios::binary);
//  assert(infile.is_open());
  // Auto correlations
  int nstations = ConPrms.number_stations();
	std::string ref_station1=ConPrms.reference_station();
	std::string ref_station2="";
  // Cross correlations
  if (ref_station1 == "") {
    Plot_data plot_data;
    set_plot_data(plot_data, ConPrms, count_channel);

    generate_auto_plots(infile, 0, nstations, plot_data, ConPrms);
    generate_cross_plots(infile, nstations, ConPrms.reference_station(), plot_data,
												ConPrms);

    plot_data_channels.push_back(plot_data);
  } else 
	
		if (ref_station2 == "") {
    // One reference station
    Plot_data plot_data;
    set_plot_data(plot_data, ConPrms, count_channel);

    generate_auto_plots(infile, 0, nstations, plot_data, ConPrms);
    generate_cross_plots(infile, nstations, ConPrms.reference_station(), plot_data,
												ConPrms);

    plot_data_channels.push_back(plot_data);
  } else {
	    // Two reference stations, 
    // devide over four plot-data's (parallel and cross)
    Plot_data plot_data1, plot_data2, plot_data3, plot_data4;
    set_plot_data(plot_data1, ConPrms, count_channel);
    set_plot_data(plot_data2, ConPrms, count_channel);
    set_plot_data(plot_data3, ConPrms, count_channel);
    set_plot_data(plot_data4, ConPrms, count_channel);

    // reset job names
    int job_name_length = plot_data1.job_name.length();
    std::string job_name_core = 
      plot_data1.job_name.substr(0, job_name_length-10);
    std::string job_name_ch1 = 
      plot_data1.job_name.substr(job_name_length-9, 4);
    std::string job_name_ch2 = 
      plot_data1.job_name.substr(job_name_length-4, 4);
    plot_data1.job_name = job_name_core+" "+job_name_ch1+" parallel";
    plot_data2.job_name = job_name_core+" "+job_name_ch1+" cross";
    plot_data3.job_name = job_name_core+" "+job_name_ch2+" parallel";
    plot_data4.job_name = job_name_core+" "+job_name_ch2+" cross";

    generate_auto_plots(infile, 0, nstations/2, plot_data1, ConPrms);
    generate_auto_plots(infile, nstations/2, nstations, plot_data3, ConPrms);

    // parallel polarisation 1
    generate_cross_plots(infile, nstations/2, 
                         ConPrms.reference_station(),
                         plot_data1, ConPrms);
    // cross polarisation 1
    generate_cross_plots(infile, nstations/2, 
                         ConPrms.reference_station(),
                         plot_data2, ConPrms);
    // cross polarisation 2
    generate_cross_plots(infile, nstations/2,
                         ConPrms.reference_station(),
                         plot_data4, ConPrms);
    // parallel polarisation 1
    generate_cross_plots(infile, nstations/2,
                         ConPrms.reference_station(),
                         plot_data3, ConPrms);

    plot_data_channels.push_back(plot_data1);
    plot_data_channels.push_back(plot_data2);
    plot_data_channels.push_back(plot_data3);
    plot_data_channels.push_back(plot_data4);
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
             ConPrms.experiment().c_str(), ConPrms.station(station).c_str(), plot_nr);
    plot_data.autos.push_back(filename);
    plot_nr++;
    
    plot(filename, nLags, title);    
  }
}

void 
Plot_generator::generate_cross_plots(std::ifstream &in,
                                     int nStations,
                                     const std::string &ref_station,
                                     Plot_data &plot_data,
																		 const Control_parameters &ConPrms) {
  if (ref_station == "") {
    // Computed all cross products
    for (int i=0; i<nStations; i++) {
      for (int j=i+1; j<nStations; j++) {
        generate_cross_plot(in, ConPrms.station(i), j, plot_data, ConPrms);
      }    
    }
  } else {
    // Computed crosses w.r.t. one reference station
    // Computed all cross products
    for (int station=0; station<nStations; station++) {
      if (ref_station != ConPrms.station(station)) {
        generate_cross_plot(in, ref_station, station, plot_data, ConPrms);
      }
    }
  }
}

void 
Plot_generator::generate_cross_plot(std::ifstream &infile,
                                    const std::string &ref_station,
                                    int station2,
                                    Plot_data &plot_data,
																		const Control_parameters &ConPrms) {
  infile.read((char *)&in[0], 2*in.size()*sizeof(double));
  fftw_execute(visibilities2lags);
  for  (int lag=0; lag<nLags; lag++) {
    magnitude[lag] = abs(out[(lag+nLags/2)%nLags])/nLags;
  }
      
  char title[80], filename[80];
  snprintf(title, 80, "Cross %s vs. %s", 
           ref_station.c_str(), 
           ConPrms.station(station2).c_str());
  snprintf(filename, 80, "%s_%s-%s_%d.png", 
           ConPrms.experiment().c_str(), 
           ref_station.c_str(), 
           ConPrms.station(station2).c_str(),
           plot_nr);
  plot_data.crosses.push_back(filename);
  plot_data.snr_crosses.push_back(signal_to_noise_ratio(out));
  plot_nr++;

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
      html_output << "<tr><td></td>"
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
      nStations /= 2;
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


  Control_parameters ConPrms;
  Log_writer_cout logg;
	
  ConPrms.initialise(argv[1], argv[2], logg);

  assert(strncmp(ConPrms.get_output_file().c_str(), "file://", 7) == 0);
  std::ifstream infile(ConPrms.get_output_file().c_str()+7, 
                       std::ios::in | std::ios::binary);
  assert(infile.is_open());

  for (int i=0; i<ConPrms.channels_size(); i++) {
    Plot_generator(infile, ConPrms, i);
  }

  print_html(ConPrms);

  return 0;
}
