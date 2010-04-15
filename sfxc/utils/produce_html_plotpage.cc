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

#include <sys/stat.h>

bool file_exists(char *filename) {
  struct stat stFileInfo;
  int result;

  // Attempt to get the file attributes
  result = stat(filename, &stFileInfo);
  return (result==0);
}

bool copy_file(char *from, char *to) {
  std::ifstream in(from);
  if (!in.is_open()) return false;
  std::ofstream out(to);
  if (!out.is_open()) return false;

  while (in.good())
    out << (char) in.get();

  return true;
}

// Generates the html-pages used for the ftp-fringe tests.
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
  if(input == NULL){
    std::cout << "Couldn't open correlator file : " << argv[2] << "\n";
    exit(1);
  }

  char *output_dir = (char*)".";
  if (argc==4)
    output_dir = argv[3];
  char *vex_file;

  {
    // Copy the vex-file:
    char *from =argv[1];
    // Basename might change from2, so we need to copy it
    char from2[strlen(from)+1]; strcpy(from2, from);
    char to[strlen(from)+strlen(output_dir)+2];
    char *vex_file_temp = basename(from2);
    vex_file = new char[strlen(vex_file_temp)+1];
    strcpy(vex_file, vex_file_temp);
    sprintf(to, "%s/%s", output_dir, vex_file);
    copy_file(from, to);
  }

  if (argc== 4) {
    // Goto the output directory
    int err = chdir(output_dir);
    // Make sure it exists
    if (err != 0) {
      std::cout << "Could not go to directory " << argv[3] << std::endl;
      return -1;
    }
  }


  // read the data in
  Fringe_info_container fringe_info(input, !update);
  if ((!update) && fringe_info.eof()) {
    std::cout << "Empty correlation file" << std::endl;
    return 1;
  }


  do {
    fringe_info.read_plots(!update);

    fringe_info.print_html(vex, vex_file);

    std::cout << "Produced html page" << std::endl;
  } while (update);

  return 0;
}
