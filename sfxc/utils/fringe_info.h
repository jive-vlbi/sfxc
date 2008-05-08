#include <vector>
#include <complex>

#include <fftw3.h>
#include <vex/Vex++.h>

#include "gnuplot_i.h"
#include "output_header.h"

class Fringe_info {
public:
  enum SPACE {
    FREQUENCY =0,
    LAG
  };

  Fringe_info() : initialised(false) {}
  
  Fringe_info(const Output_header_baseline &header,
              const std::vector< std::complex<float> > &data_freq_,
              const std::vector< std::complex<float> > &data_lag_);
  
  void plot(char *filename, char *filename_large, 
            char *title, SPACE space) const;

  float signal_to_noise_ratio() const;
  
  int max_value_offset() const;

public:
  Output_header_baseline                 header;
  std::vector< std::complex<float> >     data_freq, data_lag;
  bool                                   initialised;
};




// Container for all plots
class Fringe_info_container {
  typedef std::vector< std::vector< std::vector<Fringe_info> > > Container;

public:
  Fringe_info_container(FILE *input);

  void read_plots(bool stop_at_eof);

  void print_html(const Vex &vex);

  const Fringe_info &get_first_plot();
  const Fringe_info &get_plot(Output_header_baseline &baseline_header);

private:
  void read_data_from_file(int to_read, char * data, bool stop_at_eof);

  void set_plot(const Fringe_info &fringe_info);

  void generate_filename(char *filename,
                         char *filename_large,
                         char *title,
                         int size,
                         const Fringe_info &data);

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

  // Arrays containing one fft
  fftwf_plan fftwf_plan_;
  std::vector< std::complex<float> > data_freq, data_lag;

  // To be able to return a dummy reference
  Fringe_info empty_fringe_info;
};
