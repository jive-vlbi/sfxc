#include <vector>
#include <complex>
#include <set>

#include "sfxc_fft_float.h"
#include "utils.h"
#include <vex/Vex++.h>

#include "gnuplot_i.h"
#include "output_header.h"
#include "control_parameters.h"

class Fringe_info {
public:
  enum SPACE {
    FREQUENCY =0,
    LAG
  };
  enum VALUE {
    REAL = 0,
    IMAG,
    ABS,
    PHASE
  };

  Fringe_info() : initialised(false) {}

  Fringe_info(const Output_header_baseline &header,
              const std::vector< std::complex<float> > &data_freq_,
              const std::vector< std::complex<float> > &data_lag_);

  void plot(char *filename, char *filename_large,
            char *title, SPACE space, VALUE value, double frequency, double bandwith) const;

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
  typedef Vex::Date Date;
  typedef std::set<Fringe_info> Container;
  typedef Container::iterator   iterator;
  struct Channel{
    double frequency;
    int frequency_nr;
    int sideband;
    int polarization;
  };

public:
  Fringe_info_container(FILE *input, bool stop_at_eof);

  void read_plots(bool stop_at_eof);

  void print_html(const Vex &vex, char *vex_filename);
  void print_html_bitstatistics(const Vex &vex, const std::string &mode, std::ofstream &index_html);
  const Fringe_info &get_first_plot() const;
  const Fringe_info &get_plot(const Output_header_baseline &baseline_header) const;

  void print_diff_html(const Vex &vex,
                       const Fringe_info_container &other_info,
                       bool relative_error);

  bool eof();
private:
  void read_data_from_file(int to_read, char * data, bool stop_at_eof);
  std::string get_statistics_color(int64_t val, int64_t N);

  bool get_channels(const Vex &vex, const std::string &mode, std::vector<Channel> &channels);
  void get_bbc(const Vex &vex, std::vector<std::string> &stations, std::string &mode,
               std::vector< std::vector<int> > &bbcs, std::vector<double> &bandwiths);

  void set_plot(const Fringe_info &fringe_info);
  void process_new_bit_statistics();

  void generate_filename(char *filename,
                         char *filename_large,
                         char *title,
                         int size,
                         const Fringe_info &data,
                         const Fringe_info::SPACE space,
                         const Fringe_info::VALUE value);

  void print_auto(std::ostream &index_html, const Fringe_info &fringe_info, int bbc, 
                  double frequency, double bandwidth);

  void print_cross(std::ostream &index_html, const Fringe_info &fringe_info, 
                   double frequency, double bandwidth);

  void print_diff(std::ostream &index_html,
                  Fringe_info fringe_info1,
                  const Fringe_info &fringe_info2,
                  bool relative_error,
                  Fringe_info::SPACE space, 
                  double frequency, double bandwidth);

  // Begin and end one row of the html table
  void begin_data_row(std::ostream &index_html,
                      const std::vector<double> &frequencies,
                      const Fringe_info &fringe_info);
  void begin_data_row(std::ostream &index_html, Channel &channel);
  void end_data_row(std::ostream &index_html);

  // input file
  FILE *input;

  Container plots;

  // The global header in the data
  Output_header_global global_header;

  // Header of the last timeslice read;
  Output_header_timeslice first_timeslice_header, last_timeslice_header;

  // Bit statistics, the bitstatistics are stored in an ordered set
  struct stats_comp {
    bool operator() (const Output_header_bitstatistics& lhs, const Output_header_bitstatistics& rhs) const
    {
      if(lhs.station_nr<rhs.station_nr)
        return true;
      else if(lhs.station_nr==rhs.station_nr){
        if(lhs.frequency_nr<rhs.frequency_nr)
          return true;
        else if(lhs.frequency_nr==rhs.frequency_nr){
          if(lhs.sideband<rhs.sideband)
            return true;
          else if(lhs.sideband==rhs.sideband)
            return lhs.polarisation<rhs.polarisation;
        }
      }
      return false;
    }
  };
  typedef std::set<Output_header_bitstatistics, stats_comp> statistics_set;
  std::vector<Output_header_bitstatistics> new_statistics;
  statistics_set statistics;

  // Arrays containing one fft
  SFXC_FFT_FLOAT fft;
  std::vector< std::complex<float> > data_freq, data_lag;

  // To be able to return a dummy reference
  Fringe_info empty_fringe_info;
};
