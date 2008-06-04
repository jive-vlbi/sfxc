#include "delay_table_akima.h"

#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "usage: " << argv[0] << " <delay_file.del> <output_file>" << std::endl;
    exit(-1);
  }

  char *delay_file = argv[1];
  char *output_file = argv[2];
  Delay_table_akima delay_table;
  delay_table.open(delay_file);

  std::ofstream out(argv[2]);
  out.precision(20);
  out << delay_table;
  
}
