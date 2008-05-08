/*
 * author : N.G.H. Kruithof
 */

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
#include "log_writer_cout.h"
#include "gnuplot_i.h"
#include "utils.h"

#include <vex/Vex++.h>

#include "output_header.h"
#include "fringe_info.h"

int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  if ((argc < 3) || (argc > 5)) {
    std::cout << "usage: " << argv[0] << " [-f] <vex-file> <correlation_file> [<output_directory>]"
    << std::endl;
    exit(1);
  }

  // test for -f
  bool update = false;
  if (strcmp(argv[1], "-f") == 0) {
    update = true;
    argc--;
    argv++;
  } else {
    if (argc > 4) {
      std::cout << "usage: " << argv[0] << " [-f] <vex-file> <correlation_file> [<output_directory>]"
      << std::endl;
      exit(1);
    }
  }

  // Parse the vex file
  Vex vex;
  {
    char * vex_file = argv[1];
    std::ifstream in(vex_file);
    if (!in.is_open()) {
      std::cout << "Could not open vex file ["<<vex_file<<"]"<< std::endl;
      return false;
    }

    // parse the vex file
    if (!vex.open(vex_file)) {
      std::cout << "Could not parse vex file ["<<vex_file<<"]" << std::endl;
      return false;
    }
  }

  // open the input file
  FILE *input = fopen(argv[2], "rb");
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
  Fringe_info_container fringe_info(input);

  do {
    fringe_info.read_plots(!update);

    fringe_info.print_html(vex);

    std::cout << "Produced html page" << std::endl;
  } while (update);

  return 0;
}
