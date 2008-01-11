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
#include "data_writer_void.h"
#include <boost/shared_ptr.hpp>


int main(int argc, char*argv[]) {
#ifdef SFXC_PRINT_DEBUG
  RANK_OF_NODE = 0;
#endif

  Log_writer_cout log_writer(10);

  if (argc != 3) {
    DEBUG_MSG("Usage: " << argv[0] << " <vex_file> <ctrl_file>");
    exit(-1);
  }
  char *ctrl_file = argv[1];
  char *vex_file = argv[2];

  Control_parameters control_parameters;
  control_parameters.initialise(ctrl_file, vex_file, log_writer);
  // NGHK: TODO: get the right scan
  std::string scan = control_parameters.scan(0);
  std::string mode = control_parameters.get_vex().get_mode(scan);
  std::string station = control_parameters.station(0);
  std::string data_source = control_parameters.data_sources(station)[0];

  // Open data reader
  Data_reader_file data_reader(data_source);

  // Open input node tasklet
  Input_node_tasklet *input_node_tasklet;
  input_node_tasklet = get_input_node_tasklet(&data_reader);

  // Set track parameters
  Input_node_parameters input_node_param =
    control_parameters.get_input_node_parameters(mode, station);
  input_node_tasklet->set_parameters(input_node_param);

  { // Set delay table
    Delay_table_akima delay_table;
    const std::string &delay_file =
      control_parameters.get_delay_table_name(station);
    delay_table.open(delay_file.c_str());
    input_node_tasklet->set_delay_table(delay_table);
  }

  // Find the right starting time
  int32_t start_time = control_parameters.get_start_time().to_miliseconds();
  start_time = input_node_tasklet->goto_time(start_time);

  int32_t stop_time = control_parameters.get_stop_time().to_miliseconds();
  assert(start_time < stop_time);
  input_node_tasklet->set_stop_time(stop_time);

  { // Set data writers
    assert(control_parameters.number_frequency_channels() > 0);
    boost::shared_ptr<Data_writer> data_writer;

    int nr_integrations = (stop_time-start_time)/control_parameters.integration_time();

    data_writer = boost::shared_ptr<Data_writer>(new Data_writer_file("file://output.bin"));
    input_node_tasklet->add_data_writer(0, data_writer, nr_integrations);
    for (size_t i=1; i<control_parameters.number_frequency_channels(); i++) {
      data_writer = boost::shared_ptr<Data_writer>(new Data_writer_void());
      // Output all data
      input_node_tasklet->add_data_writer(i, data_writer, 0);
    }
  }



  // Connect to an output file
  Data_writer_file data_writer("file://test_input_node.out");
  input_node_tasklet->append_time_slice(start_time, start_time+1000,
                                        &data_writer);


  while (input_node_tasklet->has_work()) {
    input_node_tasklet->do_task();
  }

  std::cout << "Done." << std::endl;
  return 0;
}
