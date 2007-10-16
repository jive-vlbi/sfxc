#include <iostream>
#include <fstream>
#include <utils.h>
#include <Delay_table_akima.h>

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
  delay_table.open(argv[1]);
  
  std::ofstream out(argv[2]);
  out.precision(20);
  
  do {
    // In microseconds
    int64_t start_time_scan = delay_table.start_time_scan(); 
    int64_t stop_time_scan = delay_table.stop_time_scan();
    
    std::cout << start_time_scan << " \t" << stop_time_scan << std::endl;
    for (int64_t time = start_time_scan; time<stop_time_scan; time += 10000) {
      out << time << " \t" << delay_table.delay(time) << std::endl;
    }
    out << std::endl;
  } while (delay_table.initialise_next_scan());
}
