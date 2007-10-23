/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Manager_node.h>
#include <iostream>
#include <assert.h>

#include "sfxc_mpi.h"
#include "utils.h"
#include "MPI_Transfer.h"
#include <fftw3.h>

#include <Log_writer_cout.h>

Manager_node::
Manager_node(int rank, int numtasks,
    Log_writer *log_writer,
    const Control_parameters &control_parameters)
: Abstract_manager_node(rank, numtasks,
                        log_writer,
                        control_parameters),
                        manager_controller(*this) {
  assert(rank == RANK_MANAGER_NODE);

  add_controller(&manager_controller);

  get_log_writer()(1) << "Starting nodes" << std::endl;

  // initialise the log node
  //start_log_node(RANK_LOG_NODE, "file://./output.txt");
  start_log_node(RANK_LOG_NODE);
  
  // initialise the output node
  start_output_node(RANK_OUTPUT_NODE);
  set_data_writer(RANK_OUTPUT_NODE, 0, 
                  control_parameters.get_output_file());


  // Input nodes:
  int n_stations = get_control_parameters().number_stations();
  for (int input_node=0; input_node<n_stations; input_node++) {
    assert(input_node+3 != RANK_MANAGER_NODE);
    assert(input_node+3 != RANK_LOG_NODE);
    assert(input_node+3 != RANK_OUTPUT_NODE);
    assert(input_node+3 < numtasks);

    start_input_node(/*rank_nr*/ input_node+3, 
        get_control_parameters().station(input_node));
  }
  assert(n_stations > 0);


  // correlator nodes:
  if (numtasks-(n_stations+3) <
      control_parameters.number_frequency_channels()) {
    std::cout << "#correlator nodes < #freq. channels, use at least "
              << n_stations+3+control_parameters.number_frequency_channels()
              << " nodes." << std::endl
              << "Exiting now." << std::endl;
    get_log_writer()(1)
              << "#correlator nodes < #freq. channels, use at least "
              << n_stations+3+control_parameters.number_frequency_channels()
              << " nodes." << std::endl
              << "Exiting now." << std::endl;
    exit(1);
  }
  n_corr_nodes = numtasks-(n_stations+3);
  for (int correlator_nr = 0; 
       correlator_nr < n_corr_nodes; 
       correlator_nr++) {
    int correlator_rank = correlator_nr + n_stations+3;
    assert(correlator_rank != RANK_MANAGER_NODE);
    assert(correlator_rank != RANK_LOG_NODE);
    assert(correlator_rank != RANK_OUTPUT_NODE);

    start_correlator_node(/*rank_nr*/ correlator_rank);
    
    // Set up the connection to the input nodes:
    for (int station_nr=0; station_nr<n_stations; station_nr++) {
      set_TCP(input_rank(get_control_parameters().station(station_nr)), 
              correlator_nr,
              correlator_rank, station_nr);
    }

    if (control_parameters.cross_polarize()) {
      // duplicate all stations:
      for (int station_nr=0; station_nr<n_stations; station_nr++) {
        set_TCP(input_rank(get_control_parameters().station(station_nr)), 
                correlator_nr+n_corr_nodes,
                correlator_rank, station_nr+n_stations);
      }
    }


    // Set up the connection to the output node:
    set_TCP(correlator_rank, 0,
            RANK_OUTPUT_NODE, correlator_nr);
  }

  assert(number_correlator_nodes() > 0);
  state_correlator_node.resize(number_correlator_nodes());
}

Manager_node::~Manager_node() {
  for (int rank=0; rank < numtasks; rank++) {
    if ((rank != RANK_MANAGER_NODE) && 
        (rank != RANK_LOG_NODE)) {
      end_node(rank);
    }
  }
}

void Manager_node::start() {
  get_log_writer()(1) << "Manager_node::start()" << std::endl;

  initialise();
  
  status = START_NEW_SCAN;

  while (status != END_NODE) {
    process_all_waiting_messages();
    switch (status) {
      case START_NEW_SCAN:
      {
        get_log_writer() << "START_NEW_SCAN" << std::endl;
        // Set the input nodes to the proper start time
        get_log_writer() << "START_TIME:" << start_time << std::endl;
        for (size_t station=0; station < control_parameters.number_stations();
             station++) {
          int station_time = 
            input_node_get_current_time(control_parameters.station(station));
          if (station_time > start_time) {
            get_log_writer() << "START_TIME: " << start_time << std::endl;
            get_log_writer() << "STATION_TIME: (" << control_parameters.station(station) 
                             << "): " << station_time << std::endl;
            start_time = (station_time/1000)*1000;
            if (station_time%1000 != 0) start_time += 1000;
            get_log_writer() << "new START_TIME: " << start_time << std::endl;
          }
        }
        stop_time_scan =
          control_parameters.get_vex().stop_of_scan(*scans.begin()).to_miliseconds(start_day);

        for (size_t station=0; station < control_parameters.number_stations();
             station++) {
          input_node_goto_time(control_parameters.station(station),
                               start_time);
          input_node_set_stop_time(control_parameters.station(station),
                                   stop_time_scan);
        }
        status = START_CORRELATION_TIME_SLICE;
        break;
      }
      case START_CORRELATION_TIME_SLICE: 
      {
        get_log_writer() << "START_CORRELATION_TIME_SLICE" << std::endl;
        stoptime_timeslice = start_time+duration_time_slice;
        if (stop_time_scan < stoptime_timeslice) {
          stoptime_timeslice = stop_time_scan;
        }
        current_channel = 0;
        status = START_CORRELATOR_NODES_FOR_TIME_SLICE;
        break;
      }
      case START_CORRELATOR_NODES_FOR_TIME_SLICE:
      {
        get_log_writer() << "START_CORRELATOR_NODES_FOR_TIME_SLICE" << std::endl;
        bool added_correlator_node = false;
        for (size_t i=0; 
             (current_channel<control_parameters.number_frequency_channels())
             && (i<number_correlator_nodes());
             i++) {
          if (get_correlating_state(i) == READY) {
            start_next_timeslice_on_node(i);
            added_correlator_node = true;
          }
        }

        if (added_correlator_node) {
          if (current_channel == control_parameters.number_frequency_channels()) {
            status = GOTO_NEXT_TIMESLICE;
          }
        } else {
          // No correlator node added, wait for the next message
          check_and_process_message();
        }

        break;
      }
      case GOTO_NEXT_TIMESLICE:
      {
        // NGHK: todo check for new scan
        start_time = stoptime_timeslice;
        if (start_time < stop_time) {
          status = START_CORRELATION_TIME_SLICE;
        } else {
          status = STOP_CORRELATING;
        }
        break;
      }
      case STOP_CORRELATING:
      {
        // The status is set to END_NODE as soon as the output_node is ready
        MPI_Send(&slice_nr, 1, MPI_INT32, 
                 RANK_OUTPUT_NODE, MPI_TAG_OUTPUT_NODE_CORRELATION_READY,
                 MPI_COMM_WORLD);

        status = WAIT_FOR_OUTPUT_NODE;
        break;
      }
      case WAIT_FOR_OUTPUT_NODE:
      {
        // The status is set to END_NODE as soon as the output_node is ready
        check_and_process_message();
        break;
      }
      case END_NODE:
      {
        get_log_writer() << "END_NODE" << std::endl;
        break;
      }
    }
  }

  get_log_writer()(1) << "Terminating nodes" << std::endl;
}

void Manager_node::start_next_timeslice_on_node(int corr_node_nr) {
  int cross_channel = -1;
  if (control_parameters.cross_polarize()) {
    cross_channel = control_parameters.cross_polarisation(current_channel);
    assert((cross_channel == -1) || (cross_channel > current_channel));
  }

  // Initialise the correlator node
  if (cross_channel == -1) {
    get_log_writer() << "start " << start_time << ", channel " 
                     << current_channel << " to correlation node " 
                     << corr_node_nr << std::endl;
  } else {
    get_log_writer() << "start " << start_time << ", channel " 
                     << current_channel << "," 
                     << cross_channel << " to correlation node " 
                     << corr_node_nr << std::endl;
  }

  std::string channel_name =
    control_parameters.frequency_channel(current_channel);
  Correlation_parameters correlation_parameters = 
    control_parameters.
    get_correlation_parameters(*scans.begin(),
                               channel_name,
                               get_input_node_map());
  correlation_parameters.start_time = start_time;
  correlation_parameters.stop_time  = stoptime_timeslice;
  correlation_parameters.slice_nr = slice_nr;

  // Check the cross polarisation
  if (cross_channel != -1) {
    int n_stations = control_parameters.number_stations();
    int n_streams = correlation_parameters.station_streams.size();
    assert(n_stations == n_streams);
    // Add the cross polarisations
    for (int i=0; i<n_stations; i++) {
      Correlation_parameters::Station_parameters stream = 
        correlation_parameters.station_streams[i];
      stream.station_stream += n_stations;
      correlation_parameters.station_streams.push_back(stream);
    }
  }

  correlator_node_set(correlation_parameters, corr_node_nr);

  // set the input streams
  int nStations = control_parameters.number_stations();
  for (size_t station_nr=0; 
       station_nr< nStations;
       station_nr++) {
    input_node_set_time_slice(control_parameters.station(station_nr),
                              current_channel,
                              /*stream*/corr_node_nr,
                              start_time,
                              stoptime_timeslice);

    if (cross_channel != -1) {
      // Add the cross polarisation channel
      input_node_set_time_slice(control_parameters.station(station_nr),
                                cross_channel,
                                /*stream*/corr_node_nr+n_corr_nodes,
                                start_time,
                                stoptime_timeslice);
    }
  }
            
  // set the output stream
  //               autos       crosses
  int nAutos = nStations;
  int nCrosses = nStations*(nStations-1)/2;
  int nBaselines;
  if (cross_channel == -1) {
    nBaselines = nAutos + nCrosses;
  } else {
    // Doing cross polarisations
    nBaselines = 2*nAutos + 4*nCrosses;
  }
  int size_of_one_baseline = sizeof(fftw_complex)*
    (correlation_parameters.number_channels*PADDING/2+1);
  output_node_set_timeslice(slice_nr, corr_node_nr, 
                            size_of_one_baseline*nBaselines);

  set_correlating_state(corr_node_nr, CORRELATING);

  current_channel ++;
  if (control_parameters.cross_polarize()) {
    // Go to the next channel.
    int cross_channel = 
      control_parameters.cross_polarisation(current_channel);
    while ((current_channel <
            control_parameters.number_frequency_channels()) &&
           (cross_channel >= 0) && (cross_channel < current_channel)) {
      current_channel ++;
      cross_channel = 
        control_parameters.cross_polarisation(current_channel);
    }
  }
  slice_nr++;
}

void
Manager_node::initialise() {
  get_log_writer()(1) << "Initialising the Input_nodes" << std::endl;
  for (size_t station=0; 
       station<control_parameters.number_stations(); station++) {
    // setting the first data-source of the first station
    const std::string &station_name = control_parameters.station(station);
    std::string filename = control_parameters.data_sources(station_name)[0];
    set_data_reader(input_rank(station_name), 0, filename);
  }

  // Get a list of all scan names
  control_parameters.get_vex().get_scans(std::back_inserter(scans));

  // Send the track parameters
  // PRECONDITION: they are the same for the entire experiment
  get_log_writer() << "Set Track_parameters" << std::endl;
  for (size_t station=0; 
       station<control_parameters.number_stations(); station++) {
    const std::string &mode = 
      control_parameters.get_vex().get_mode(*scans.begin());
    const std::string &station_name = 
      control_parameters.station(station);
    const std::string &track = 
      control_parameters.get_vex().get_track(mode, station_name);

    Track_parameters track_param =
      control_parameters.get_track_parameters(track);
    input_node_set(station_name, track_param);
  }

  // Send the delay tables:
  get_log_writer() << "Set delay_table" << std::endl;
  for (size_t station=0; 
       station<control_parameters.number_stations(); station++) {
    Delay_table_akima delay_table;
    const std::string &station_name = control_parameters.station(station);
    const std::string &delay_file = 
      control_parameters.get_delay_table_name(station_name);
    delay_table.open(delay_file.c_str());

    correlator_node_set_all(delay_table, station_name);
  }

  start_day  = control_parameters.get_start_time().day;
  start_time = control_parameters.get_start_time().to_miliseconds();
  stop_time  = 
    control_parameters.get_stop_time().to_miliseconds(start_day);

  get_log_writer()(2) << "start_day  : " << start_day << std::endl;
  get_log_writer()(2) << "start_time : " << start_time << std::endl;
  get_log_writer()(2) << "stop_time  : " << stop_time << std::endl;

  {  // Iterate over all the scans to find the first scan to correlate
    const Vex &vex = control_parameters.get_vex();

    const std::string &mode = 
      control_parameters.get_vex().get_mode(*scans.begin());
    while (!scans.empty()) {
      // assume the same mode, hence the same track parameters
      assert(mode == control_parameters.get_vex().get_mode(*scans.begin()));

      if (vex.stop_of_scan(*scans.begin()).to_miliseconds(start_day) > 
          start_time) {
        break;
      }
      scans.erase(scans.begin());
    }
  }
  assert(!scans.empty());
  
  slice_nr  = 0;

  get_log_writer()(1) << "Initialisation finished" << std::endl;

  get_log_writer()(2) << "start scan : " << *scans.begin() << std::endl;

  get_log_writer()(2) << "Starting correlation" << std::endl;
}

void Manager_node::end_correlation() {
  assert(status == WAIT_FOR_OUTPUT_NODE);
  status = END_NODE;
}
