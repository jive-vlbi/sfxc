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

  if ((argc < 4) || (argc > 5)) {
    std::cout << "usage: " << argv[0] << " <vex-file> <correlation_file1> <correlation_file2> [<output_directory>]"
    << std::endl;
    exit(1);
  }

  char * vex_file = argv[1];
  char * cor_file1 = argv[2];
  char * cor_file2 = argv[3];
  char * output_dir = NULL;
  if (argc == 5)
    output_dir = argv[4];

  // Parse the vex file
  Vex vex;
  {
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
  FILE *input1 = fopen(cor_file1, "rb");
  FILE *input2 = fopen(cor_file2, "rb");
  assert(input1 != NULL);
  assert(input2 != NULL);

  if (output_dir != NULL) {
    // Goto the output directory
    int err = chdir(output_dir);
    // Make sure it exists
    if (err != 0) {
      std::cout << "Could not go to directory " << output_dir << std::endl;
      return -1;
    }
  }


  // read the data in
  Fringe_info_container fringe_info1(input1);
  Fringe_info_container fringe_info2(input2);

  fringe_info1.read_plots(true);
  fringe_info2.read_plots(true);

  fringe_info1.print_diff_html(vex, fringe_info2, false);

  std::cout << "Compared output files" << std::endl;

  return 0;
}
