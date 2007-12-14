/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#include "utils.h"
#include "input_node_tasklet.h"
#include "log_writer_cout.h"
#include "control_parameters.h"
#include "data_reader_file.h"
#include "data_writer_file.h"


int main(int argc, char*argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  Log_writer_cout log_writer(10);

  assert(argc == 3);
  char *ctrl_file = argv[1];
  char *vex_file = argv[2];

  Control_parameters control_parameters;
  control_parameters.initialise(ctrl_file, vex_file, log_writer);

  std::string station = control_parameters.station(0);

  std::string data_source = control_parameters.data_sources(station)[0];
  Data_reader_file data_reader(data_source);

  Input_node_tasklet *input_node_tasklet;
  input_node_tasklet = get_input_node_tasklet(&data_reader);

  // Find the right starting time
  int32_t start_time = control_parameters.get_start_time().to_miliseconds();
  input_node_tasklet->goto_time(start_time);

  Data_writer_file data_writer("file://test_input_node.out");
  input_node_tasklet->append_time_slice(start_time, start_time+1000,
                                        &data_writer);

  input_node_tasklet->do_task();

  std::cout << "Done." << std::endl;
  return 0;
}
