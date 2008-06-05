#include <vector>
#include <complex>
#include <set>

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

  bool operator==(const Fringe_info &other) const;
  bool operator<(const Fringe_info &other) const;
public:
  Output_header_baseline                 header;
  std::vector< std::complex<float> >     data_freq, data_lag;
  bool                                   initialised;
};




// Container for all plots
class Fringe_info_container {
  typedef std::set<Fringe_info> Container;
  typedef Container::iterator   iterator;

public:
  Fringe_info_container(FILE *input);

  void read_plots(bool stop_at_eof);

  void print_html(const Vex &vex, char *vex_filename);

  const Fringe_info &get_first_plot() const;
  const Fringe_info &get_plot(const Output_header_baseline &baseline_header) const;

  void print_diff_html(const Vex &vex,
                       const Fringe_info_container &other_info,
                       bool relative_error);

private:
  void read_data_from_file(int to_read, char * data, bool stop_at_eof);

  void set_plot(const Fringe_info &fringe_info);

  void generate_filename(char *filename,
                         char *filename_large,
                         char *title,
                         int size,
                         const Fringe_info &data);

  void print_auto(std::ostream &index_html,
                  const Fringe_info &fringe_info);

  void print_cross(std::ostream &index_html,
                   const Fringe_info &fringe_info);

  void print_diff(std::ostream &index_html,
                  Fringe_info fringe_info1,
                  const Fringe_info &fringe_info2,
                  bool relative_error,
                  Fringe_info::SPACE space);

  // Begin and end one row of the html table
  void begin_data_row(std::ostream &index_html,
                      const std::vector<double> &frequencies,
                      const Fringe_info &fringe_info);
  void end_data_row(std::ostream &index_html);

  // input file
  FILE *input;

  Container plots;

  // The global header in the data
  Output_header_global global_header;

  // Header of the last timeslice read;
  Output_header_timeslice first_timeslice_header, last_timeslice_header;

  // Arrays containing one fft
  fftwf_plan fftwf_plan_;
  std::vector< std::complex<float> > data_freq, data_lag;

  // To be able to return a dummy reference
  Fringe_info empty_fringe_info;
};
