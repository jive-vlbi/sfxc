#include <iostream>
#include <fstream>

#include "utils.h"
#include "delay_table_akima.h"
#include "control_parameters.h"

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
  std::cout << "about to open " << argv[1] << "\n";
  delay_table.open(argv[1]);
  std::cout << "about to open " << argv[2] << "\n";
  std::ofstream out(argv[2]);
  out.precision(20);

  int scan = 1;
  do {
    std::cout << "read scan " << scan << "\n";
    Time start_time_scan = delay_table.start_time_scan();
    Time stop_time_scan = delay_table.stop_time_scan();

    std::cout << scan 
              << " \t" << start_time_scan
              << " \t" << stop_time_scan
              << std::endl;
    Time step = Time(10000.);
    for (Time time = start_time_scan; time < stop_time_scan; time += step) {
      out << (int64_t)time.get_time_usec() << " \t" << delay_table.delay(time) << std::endl;
    }
    out << std::endl;
    scan ++;
  } while (delay_table.initialise_next_scan());
}
