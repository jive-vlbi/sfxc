/*
 * author : N.G.H. Kruithof
 */

#include <fstream>

#include "output_header.h"
#include "fringe_info.h"

int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  if (argc != 8) {
    std::cout << "Usage: " << argv[0] << " <cor-file>" 
              << " <ch_nr> <sideband> <st_nr1> <pol1> <st_nr2> <pol2>" << std::endl;
    exit(-1);
  }

  Output_header_baseline baseline_header;
  baseline_header.frequency_nr  = atoi(argv[2]);
  baseline_header.sideband      = atoi(argv[3]);
  baseline_header.station_nr1   = atoi(argv[4]);
  baseline_header.polarisation1 = atoi(argv[5]);
  baseline_header.station_nr2   = atoi(argv[6]);
  baseline_header.polarisation2 = atoi(argv[7]);

  // open the input file
  FILE *input = fopen(argv[1], "rb");
  assert(input != NULL);

  if (argc== 4) {
    // Goto the output directory
    int err = chdir(argv[3]);
    // Make sure it exists
    if (err != 0) {
      std::cout << "Could not go to directory " << argv[3] << std::endl;
      return -1;
    }
  }

  // read the data in
  Fringe_info_container fringes(input);

  std::ofstream out("baseline.txt");
  out << "# fringe_pos, phase (max), ampl (max), phase (center), ampl (center),  snr, weight" << std::endl;

  do {
    fringes.read_plots(/* stop at eof */ true);

    const Fringe_info &fringe_info = fringes.get_plot(baseline_header);

    if (fringe_info.initialised) {
      int fringe_pos = fringe_info.max_value_offset();
      int center_pos = fringe_info.data_lag.size()/2+1;
      out << fringe_pos << " \t"
          << std::arg(fringe_info.data_lag[fringe_pos]) << " \t"
          << std::abs(fringe_info.data_lag[fringe_pos]) << " \t"
          << std::arg(fringe_info.data_lag[center_pos]) << " \t"
          << std::abs(fringe_info.data_lag[center_pos]) << " \t"
          << fringe_info.signal_to_noise_ratio() << " \t"
          << fringe_info.header.weight << std::endl;
//       std::cout << "weight: " << fringe_info.header.weight << std::endl;
    } else {
      out << std::endl;
    }
        

  } while (!feof(input));

  return 0;
}
