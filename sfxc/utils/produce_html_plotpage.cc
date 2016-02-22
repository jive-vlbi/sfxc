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
#include <libgen.h>
#include <complex>
#include <getopt.h> 
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

void usage(char *argv[]){
  std::cout << "Usage: " << argv[0] << " [options] <vex-file> <correlation_file> [<output_directory>]\n"
            << "       Options : -h, --help, Print this message\n"
            << "                 -f, --monitor, Don't stop reading at EOF\n"
            << "                 -s, --setup-station [STATION CODE], Set setup station\n";
}

// Generates the html-pages used for the ftp-fringe tests.
int main(int argc, char *argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif
  // Get options
  struct option options[] = {{"monitor",  no_argument,       0, 'f'},
                             {"help", no_argument, 0, 'h'},
                             {"setup-station",    required_argument, 0, 's'},
                             {0, 0, 0, 0}};
  int c;
  bool update = false;
  std::string setup_station = "";
  while(true){
    int option_index = 0;
    c = getopt_long (argc, argv, "hfs:", options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
     break;

    switch (c){
    case 'h':
      usage(argv);
      exit(0);
      break;
    case 'f':
      update = true;
      break;
    case 's':
      setup_station = optarg;
      break;
    default:
      std::cerr << "Error : invalid option\n";
      usage(argv);
      exit(1);
    }
  }
  int n_arguments = (argc - optind);
  if((n_arguments !=2) && (n_arguments != 3)){
    std::cerr << "Invalid number of arguments\n";
    usage(argv);
    exit(1);
  }

  // Parse the vex file
  Vex vex;
  {
    char * vex_file = argv[optind];
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
  FILE *input = fopen(argv[optind+1], "rb");
  if(input == NULL){
    std::cout << "Couldn't open correlator file : " << argv[optind+1] << "\n";
    exit(1);
  }

  char *output_dir = (char*)".";
  if (optind < argc)
    output_dir = argv[optind+2];
  char *vex_file;

  {
    // Copy the vex-file:
    char *from =argv[optind];
    // Basename might change from2, so we need to copy it
    char from2[strlen(from)+1]; strcpy(from2, from);
    char to[strlen(from)+strlen(output_dir)+2];
    char *vex_file_temp = basename(from2);
    vex_file = new char[strlen(vex_file_temp)+1];
    strcpy(vex_file, vex_file_temp);
    sprintf(to, "%s/%s", output_dir, vex_file);
    copy_file(from, to);
  }

  if (n_arguments == 3) {
    // Goto the output directory
    int err = chdir(output_dir);
    // Make sure it exists
    if (err != 0) {
      std::cout << "Could not go to directory " << argv[optind+2] << std::endl;
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

    fringe_info.print_html(vex, vex_file, setup_station);

    std::cout << "Produced html page" << std::endl;
  } while (update);

  return 0;
}
