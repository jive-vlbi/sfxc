#include <iostream>
#include <fstream>
#include <getopt.h>

#include "utils.h"
#include "delay_table_akima.h"
#include "control_parameters.h"

void usage(const char *name){
  std::cout << "Usage: " << name << " [options] <delay-table> <plot-file>\n";
  std::cout << "Options : -h, --help                  Print this help message\n";
  std::cout << "          -n, --nr-interpolate        The number of interpolated points to\n"
            << "                                      print between two rows of the delay\n"
            << "                                      table\n";
}

//converts a binary delay table to an interpolated ascii table
//Usage: plot_delay_table [options] <delay-table> <plot-file>
//
//each row contains a value for the time and the delay. 
//Scans are separated with an extra newline, which makes gnuplot draw a new curve.
int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif
  int n_interpol = 9;
  int c;

  while (1)
    {
      static struct option long_options[] =
        {
          {"help",    no_argument,       0, 'h'},
          {"number-points",    required_argument, 0, 'n'},
          {0, 0, 0, 0}
        };
      int option_index = 0;

      c = getopt_long (argc, argv, "hn:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 'h':
          usage(argv[0]);
          exit(0);
          break;
        case 'n':
          n_interpol = atoi(optarg);
          break;
        case '?':
          exit(1);
          break;

        default:
          usage(argv[0]);
          exit(1);
        }
    }

  if(argc-optind != 2){
    std::cout << "Error, invalid number of arguments!\n\n";
    usage(argv[0]);
    exit(1);
  }

  Delay_table delay_table;
  delay_table.open(argv[optind]);
  std::ofstream out(argv[optind+1]);
  out.precision(20);

  int scan = 1;
  do {
    std::cout << "read scan " << scan << "\n";
    Time start_time_scan = delay_table.start_time_scan();
    Time stop_time_scan = delay_table.stop_time_scan();
    Time dt = delay_table.stop_time_scan() - delay_table.start_time_scan();
    Delay_table_akima akima  = delay_table.create_akima_spline(start_time_scan, dt);

    std::cout << scan 
              << " \t" << start_time_scan
              << " \t" << stop_time_scan
              << std::endl;
    Time onesec = Time(1000000.);
    Time step = Time(1000000./(n_interpol+1));
    for (Time t0 = start_time_scan; t0 < stop_time_scan; t0 += onesec) {
      for (int i=0;i<n_interpol+1;i++){
        Time time = t0 + step*i;
        out << time << ", usec = " << (int64_t)time.get_time_usec();
        for(int j = 0 ; j < akima.n_phase_centers(); j++)
          out << " \t(" << akima.delay(time, j) 
              << ", " << akima.phase(time, j)
              << ", " << akima.amplitude(time, j) << ")";
        out << std::endl;
      }
    }
    out << std::endl;
    scan ++;
  } while (delay_table.initialise_next_scan());
}
