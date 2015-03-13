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

  Delay_table delay_table;
  delay_table.open(argv[1]);
  delay_table.set_clock_offset(0., 0., 0., 0.);
  std::ofstream out(argv[2]);
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
    Time step = Time(100000.);
    for (Time time = start_time_scan; time < stop_time_scan; time += step) {
      out << time << ", usec = " << (int64_t)time.get_time_usec();
      for(int j = 0 ; j < akima.n_phase_centers(); j++)
        out << " \t(" << akima.delay(time, j) 
            << ", " << akima.phase(time, j)
            << ", " << akima.amplitude(time, j) << ")";
      out << std::endl;
    }
    out << std::endl;
    scan ++;
  } while (delay_table.initialise_next_scan());
}
