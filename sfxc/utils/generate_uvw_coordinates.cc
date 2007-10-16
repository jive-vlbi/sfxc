#include <iostream>
#include <fstream>
#include <Uvw_model.h>


int main(int argc, char *argv[]) {
  if (argc != 6) {
    std::cout << "Usage: " << argv[0]
              << " <delay-file> <start> <stop> <integration-time> <output-file>" 
              << std::endl;
    exit(1);
  }
  
  Uvw_model model;
  model.open(argv[1]);

  int64_t start_time=strtoll(argv[2], 0, 10);
  int64_t stop_time=strtoll(argv[3], 0, 10);
  double integration_time; sscanf(argv[4], "%lf", &integration_time);
  
  std::cout << start_time << " "
            << stop_time << " "
            << integration_time << std::endl;
  std::ofstream output(argv[5]);

  model.uvw_values(output, start_time, stop_time, integration_time);
}
