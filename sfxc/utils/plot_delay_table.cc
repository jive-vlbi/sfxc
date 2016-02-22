#include <iostream>
#include <fstream>
#include <getopt.h>
#include <vex/Vex++.h>

#include "utils.h"
#include "delay_table_akima.h"
#include "control_parameters.h"

void usage(const char *name){
  std::cout << "Usage: " << name << " [options] <delay-table> <plot-file>\n";
  std::cout << "Options : -h, --help                  Print this help message\n"
            << "          -c, --clocks <vex file>     Use clock offsets from vex file\n"
            << "          -n, --nr-interpolate <nr>   The number of interpolated points to\n"
            << "                                      print between two rows of the delay\n"
            << "                                      table\n";
}

std::string
get_station_name(const char *delay_table){
  FILE *delay = fopen(delay_table, "r");
  if(delay == NULL){
    std::cout << "Error, could not open delay table: " << delay_table << "\n";
    exit(1);
  }
  int header_size;
  size_t n = fread(&header_size, sizeof(int), 1, delay);
  if(n != 1){
    std::cout << "Error premature end of delay file\n";
    exit(1);
  }
  char header[header_size];
  n = fread(&header[0], 1, header_size, delay);
  if(n != header_size){
    std::cout << "Error premature end of delay file\n";
    exit(1);
  }
  fclose(delay);
  return header;
}

void update_clocks(Delay_table &delay_table, const Vex &vex, std::string &station_name, Time scan_start){
  const Vex::Node &root = vex.get_root_node();
  Vex::Node::const_iterator clock = root["STATION"][station_name]["CLOCK"];
  if (clock == root["STATION"][station_name]->end()) {
    std::cout << "Error: Cannot find $CLOCK reference in vexfile\n";
    exit(1);
  }
  const std::string &clock_name = clock->to_string();
  if (root["CLOCK"][clock_name] == root["CLOCK"]->end()) {
    std::cout << "Error: Cannot find " << clock_name << " in $CLOCK block\n";
    exit(1);
  }
  clock = root["CLOCK"][clock_name]["clock_early"];
  if (clock == root["CLOCK"][clock_name]->end()) {
    std::cout << "Error: Cannot find clock_early entry for " << clock_name << " in vexfile\n";
    exit(1);
  }
  Time start, epoch;
  double offset = 0.0, rate = 0.0;
  for (clock = root["CLOCK"][clock_name]->begin("clock_early");
       clock != root["CLOCK"][clock_name]->end("clock_early"); clock++) {
    if (scan_start < Time(clock[0]->to_string()))
      continue;
    if (start > Time(clock[0]->to_string()))
      continue;
    start = Time(clock[0]->to_string());
    offset = clock[1]->to_double();
    rate = 0.0;
    if (clock->size() > 3) {
      rate = clock[3]->to_double() / 1e6;
      epoch = Time(clock[2]->to_string());
    }
  }
  if (start == Time()) {
    std::cout << "Error: Couldn't find valid clock_early for scan\n";
    exit(1);
  }

  // To allow large clock offsets, the reader time is adjusted
  const double max_offset = 1000000.;
  double reader_offset = round(offset / max_offset) * max_offset;
  offset = (offset - reader_offset) * 1e-6; // convert to microseconds 
  // Add the clock offsets to the delay table
  delay_table.set_clock_offset(offset, start, rate, epoch);
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
  std::string vexfile;
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

      c = getopt_long (argc, argv, "hn:c:",
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
        case 'c':
          vexfile = std::string(optarg);
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
  
  // Get station name from delay table
  std::string station_name = get_station_name(argv[optind]);
  
  // Open vex file if we were give one
  Vex vex;
  if (vexfile != std::string()){
    vex.open(vexfile.c_str());
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
    // If we were given a vex file, add the current clock offset to the model
    if (vexfile != std::string()){
      update_clocks(delay_table, vex, station_name, start_time_scan);
    }
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
