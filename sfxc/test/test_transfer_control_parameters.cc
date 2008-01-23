/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <iostream>
#include <assert.h>

#include "types.h"
#include "delay_table_akima.h"
#include "mpi_transfer.h"

#include "utils.h"
#include "log_writer_cout.h"

Log_writer_cout log_writer;

//#define VERBOSE

void check_control_parameters(int rank,
                              Control_parameters &control_parameters) {
  std::map<std::string, int> station_streams;
  { // Get station map:
    for (size_t station_nr=0;
         station_nr<control_parameters.number_stations();
         station_nr++) {
      const std::string &station_name =
        control_parameters.station(station_nr);
      station_streams[station_name] = station_nr;
    }
  }

  Vex::Node::const_iterator freqs =
    control_parameters.get_vex().get_root_node()["FREQ"];
  std::string channel_name =
    freqs->begin()["chan_def"][4]->to_string();

  if (rank==0) {
    MPI_Transfer mpi_transfer;


    // Get input_node_parameters for every scan x station
    std::vector<std::string> scans;
    control_parameters.get_vex().get_scans(std::back_inserter(scans));
    for (size_t i=0; i<scans.size(); i++) {
      // Get all stations for a certain scan
      std::vector<std::string> stations;
      control_parameters.get_vex().get_stations(scans[i], std::back_inserter(stations));
      for (size_t j=0; j<stations.size(); j++) {
        const std::string &mode =
          control_parameters.get_vex().get_mode(scans[i]);
        Input_node_parameters input_node_param =
          control_parameters.get_input_node_parameters(mode, stations[j]);
        // Sending data
        mpi_transfer.send(input_node_param, 1);
#ifdef VERBOSE

        if (j==0) {
          std::cout << "SENDING: " << input_node_param << std::endl;
          sleep(1);
        }
#endif

      }

      // Check the correlation parameters
      Correlation_parameters correlation_param =
        control_parameters.
        get_correlation_parameters(control_parameters.scan(i),
                                   channel_name,
                                   station_streams);
      mpi_transfer.send(correlation_param, 1);

#ifdef VERBOSE

      std::cout << "SENDING: " << correlation_param << std::endl;
      sleep(1);
#endif

    }

  } else { // Receiving side
    MPI_Status status;
    MPI_Transfer mpi_transfer;

    // Get input_node_parameters for every scan x station
    std::vector<std::string> scans;
    control_parameters.get_vex().get_scans(std::back_inserter(scans));
    for (size_t i=0; i<scans.size(); i++) {
      // Get all stations for a certain scan
      std::vector<std::string> stations;
      control_parameters.get_vex().get_stations(scans[i], std::back_inserter(stations));
      for (size_t j=0; j<stations.size(); j++) {
        const std::string &mode =
          control_parameters.get_vex().get_mode(scans[i]);
        Input_node_parameters input_node_param =
          control_parameters.get_input_node_parameters(mode, stations[j]);

        Input_node_parameters input_node_param2 =
          control_parameters.get_input_node_parameters(mode, stations[j]);
        // Double extraction
        assert(input_node_param == input_node_param2);
        // copy constructor
        input_node_param2 = input_node_param;
        assert(input_node_param == input_node_param2);
        // sending data
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        mpi_transfer.receive(status, input_node_param2);

#ifdef VERBOSE

        if (j==0) {
          std::cout << "RECEIVING: " << input_node_param2 << std::endl;
        }
#endif
        assert(input_node_param == input_node_param2);
      }

      // Check the correlation parameters
      Correlation_parameters correlation_param =
        control_parameters.get_correlation_parameters(control_parameters.scan(i),
            channel_name,
            station_streams);
      // Double extraction
      Correlation_parameters correlation_param2 =
        control_parameters.get_correlation_parameters(control_parameters.scan(i),
            channel_name,
            station_streams);
      assert(correlation_param == correlation_param2);
      // Copy constructor
      correlation_param2 = correlation_param;
      assert(correlation_param == correlation_param2);
      // sending data
      MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      mpi_transfer.receive(status, correlation_param2);
      assert(correlation_param == correlation_param2);

#ifdef VERBOSE
      std::cout << "RECEIVING: " << correlation_param2 << std::endl;
      sleep(1);
#endif

    }
  }
}

void check_delay_table(int rank, const std::string &filename_delay_table) {
  // Read delay_table:
  Delay_table_akima delayTable;
  delayTable.open(filename_delay_table.c_str());
  int sn = 134; // Some random value

  MPI_Transfer transfer;
  if (rank==0) {
    { // assignment
      Delay_table_akima delayTable2 = delayTable;
      assert(delayTable == delayTable2);
    }

    { // copy constructor
      Delay_table_akima delayTable2(delayTable);
      assert(delayTable == delayTable2);
    }

    transfer.send(delayTable,sn,1);
  } else {
    Delay_table_akima delayTable2;
    MPI_Status status;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    int sn2;
    transfer.receive(status,delayTable2,sn2);
    assert(delayTable == delayTable2);
    assert(sn == sn2);
  }
}


int main(int argc, char *argv[]) {
  // MPI
  int numtasks, rank;
  //initialisation

  int stat = MPI_Init(&argc,&argv);
  if (stat != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, stat);
  }

  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  assert(argc==3);

  char            *ctrl_file = argv[1];
  char            *vex_file = argv[2];

  Control_parameters parameters;
  parameters.initialise(ctrl_file, vex_file, std::cout);

  check_control_parameters(rank, parameters);

  for (size_t station_nr=0;
       station_nr<parameters.number_stations();
       station_nr++) {
    MPI_Barrier( MPI_COMM_WORLD );
    std::string delay_table =
      parameters.get_delay_table_name(parameters.station(station_nr));
    check_delay_table(rank, delay_table);
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
