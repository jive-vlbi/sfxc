#include <iostream>
#include <fstream>

#include "utils.h"
#include "delay_table_akima.h"
#include "control_parameters.h"

typedef Control_parameters::Date Date;

//converts a binary delay table to an interpolated ascii table
//Usage: plot_delay_table <delay-table> <plot-file>
//
//each row contains a value for the time and the delay. 
//Scans are separated with an extra newline, which makes gnuplot draw a new curve.
int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  if (argc != 3) {
    std::cout << "usage: " << argv[0] << " <delay-table> <plot-file>"
    << std::endl;
    exit(1);
  }

  Delay_table_akima delay_table;
  delay_table.open(argv[1]);

  std::ofstream out(argv[2]);
  out.precision(20);

  int scan = 1;
  do {
    // In microseconds
    int64_t start_time_scan = delay_table.start_time_scan();
    int64_t stop_time_scan = delay_table.stop_time_scan();

    // year and day are set to zero
    std::cout << scan 
              << " \t" << Date(0,0,start_time_scan/1000000).to_string()
              << " \t" << Date(0,0,stop_time_scan/1000000).to_string()
              << std::endl;
    for (int64_t time = start_time_scan; time<stop_time_scan; time += 10000) {
      out << time << " \t" << delay_table.delay(time) << std::endl;
    }
    out << std::endl;
    scan ++;
  } while (delay_table.initialise_next_scan());
}
