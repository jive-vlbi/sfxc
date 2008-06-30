/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "manager_node.h"
#include "sfxc_mpi.h"
#include "utils.h"
#include "mpi_transfer.h"
#include "log_writer_cout.h"

#include <iostream>
#include <iomanip>
#include <fftw3.h>

Manager_node::
Manager_node(int rank, int numtasks,
             Log_writer *log_writer,
             const Control_parameters &control_parameters)
    : Abstract_manager_node(rank, numtasks,
                            log_writer,
                            control_parameters),
    manager_controller(*this),
    integration_slice_nr(0),
    current_scan(0)
/**/ {
  SFXC_ASSERT(rank == RANK_MANAGER_NODE);

  add_controller(&manager_controller);

  get_log_writer()(1) << "Starting nodes" << std::endl;

  // initialise the log node
  //start_log_node(RANK_LOG_NODE, "file://./output.txt");
  start_log_node(RANK_LOG_NODE);

  // initialise the output node
  start_output_node(RANK_OUTPUT_NODE);
  set_data_writer(RANK_OUTPUT_NODE, 0,
                  control_parameters.get_output_file());

  { // Send the global header
    Output_header_global output_header;
    output_header.header_size = sizeof(Output_header_global);


    strcpy(output_header.experiment,control_parameters.experiment().c_str());      // Name of the experiment
    Control_parameters::Date start = control_parameters.get_start_time();
    output_header.start_year = start.year;       // Start year of the experiment
    output_header.start_day = start.day;        // Start day of the experiment (day of year)
    output_header.start_time = start.to_miliseconds()/1000;
    // Start time of the correlation in seconds since
    // midnight
    output_header.number_channels = control_parameters.number_channels();  // Number of frequency channels
    // 3 bytes left:
    int int_time_tmp=0;
    int int_time_count=0;
    int_time_tmp = control_parameters.integration_time();// Integration time: 2^integration_time seconds
    while (int_time_tmp > 1000) {
      int_time_tmp /= 2;
      int_time_count++;
    }
    while (int_time_tmp < 1000) {
      int_time_tmp *= 2;
      int_time_count--;
    }
    output_header.integration_time = (int8_t)(int_time_count);
    output_header.polarisation_type =
      control_parameters.polarisation_type_for_global_output_header();
    output_header.empty[0] = 0;
    output_header.empty[1] = 0;

    output_node_set_global_header((char *)&output_header,
                                  sizeof(Output_header_global));
  }

  // Input nodes:
  int n_stations = get_control_parameters().number_stations();
  for (int input_node=0; input_node<n_stations; input_node++) {
    SFXC_ASSERT(input_node+3 != RANK_MANAGER_NODE);
    SFXC_ASSERT(input_node+3 != RANK_LOG_NODE);
    SFXC_ASSERT(input_node+3 != RANK_OUTPUT_NODE);
    SFXC_ASSERT(input_node+3 < numtasks);

    start_input_node(/*rank_nr*/ input_node+3,
                                 get_control_parameters().station(input_node));
  }
  SFXC_ASSERT(n_stations > 0);

  // correlator nodes:
  if (numtasks-(n_stations+3) - control_parameters.number_correlation_cores_per_timeslice(get_current_mode()) < 0) {
    std::cout << "#correlator nodes < #freq. channels, use at least "
    << n_stations+3+control_parameters.number_correlation_cores_per_timeslice(get_current_mode())
    << " nodes." << std::endl
    << "Exiting now." << std::endl;
    get_log_writer()(1)
    << "#correlator nodes < #freq. channels, use at least "
    << n_stations+3+control_parameters.number_correlation_cores_per_timeslice(get_current_mode())
    << " nodes." << std::endl
    << "Exiting now." << std::endl;
    exit(1);
  }
  n_corr_nodes = numtasks-(n_stations+3);

  std::vector<MPI_Request> pending_requests;
  int numrequest;
	if( control_parameters.cross_polarize() ){
			numrequest = (n_stations*2+1) * n_corr_nodes;
	}else{
			numrequest = (n_stations+1) * n_corr_nodes;
	}
	pending_requests.resize(numrequest);
	int currreq=0;
  for (int correlator_nr = 0;
       correlator_nr < n_corr_nodes;
       correlator_nr++) {
    int correlator_rank = correlator_nr + n_stations+3;
    SFXC_ASSERT(correlator_rank != RANK_MANAGER_NODE);
    SFXC_ASSERT(correlator_rank != RANK_LOG_NODE);
    SFXC_ASSERT(correlator_rank != RANK_OUTPUT_NODE);

    start_correlator_node(/*rank_nr*/ correlator_rank);

    // Set up the connection to the input nodes:
    for (int station_nr=0; station_nr<n_stations; station_nr++) {
    	connect_to(input_rank(get_control_parameters().station(station_nr)),
								 correlator_nr,
								 correlator_rank, station_nr,
								 input_node_cnx_params_[station_nr]->ip_port_, correlator_rank, &pending_requests[currreq++] );
    }

    if (control_parameters.cross_polarize()) {
      // duplicate all stations:

      for (int station_nr=0; station_nr<n_stations; station_nr++) {
        connect_to(input_rank(get_control_parameters().station(station_nr)),
									 correlator_nr+n_corr_nodes,
									 correlator_rank,
									 station_nr+n_stations,
									 input_node_cnx_params_[station_nr]->ip_port_,
									 correlator_rank, &pending_requests[currreq++] );
      }
    }

		//currreq++;
    // Set up the connection to the output node:

    connect_writer_to(
									 correlator_rank, 0,
									 RANK_OUTPUT_NODE, correlator_nr,
									 output_node_cnx_params_[0]->ip_port_,
									 correlator_rank, &pending_requests[currreq++] );
	}

  // We simply sum all of the number of connexion eshtablished
  std::vector<MPI_Status> pending_status;
  pending_status.resize( currreq );

  MPI_Waitall( currreq, &pending_requests[0], &pending_status[0]);
	std::cout << "All the connexion are established ! " << std::endl;
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

  PROGRESS_MSG("start correlating");

  initialise();
  status = START_NEW_SCAN;
  while (status != END_NODE) {
		process_all_waiting_messages();

    switch (status) {
      case START_NEW_SCAN: {
        // set track information
        initialise_scan(control_parameters.scan(current_scan));

        // Set the input nodes to the proper start time
        for (size_t station=0; station < control_parameters.number_stations();
             station++) {
          int station_time =
            input_node_get_current_time(control_parameters.station(station));
          if (station_time >
              start_time + integration_slice_nr*integration_time()) {
            integration_slice_nr =
              (station_time-start_time)/integration_time();
          }
        }

        // Check whether the new start time is before the stop time
        get_log_writer() << "START_TIME: " << start_time << std::endl;
        if (stop_time <= start_time) {
          status = STOP_CORRELATING;
          break;
        }
        for (size_t station=0; station < control_parameters.number_stations();
             station++) {
          input_node_set_time(control_parameters.station(station),
                              start_time +
                              integration_slice_nr*integration_time(),
                              stop_time_scan);
        }
        status = START_CORRELATION_TIME_SLICE;
        break;
      }
      case START_CORRELATION_TIME_SLICE: {
        current_channel = 0;
        current_correlator_node = 0;
        status = START_CORRELATOR_NODES_FOR_TIME_SLICE;
        break;
      }
      case START_CORRELATOR_NODES_FOR_TIME_SLICE: {
        bool added_correlator_node = false;
#ifdef SFXC_DETERMINISTIC

        if (correlator_node_ready[current_correlator_node]) {
          set_correlator_node_ready(current_correlator_node, false);
          start_next_timeslice_on_node(current_correlator_node);

          added_correlator_node = true;
        }
#else
        if (!ready_correlator_nodes.empty()) {
          start_next_timeslice_on_node(ready_correlator_nodes.front());
          ready_correlator_nodes.pop();
          added_correlator_node = true;
        }
#endif

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
      case GOTO_NEXT_TIMESLICE: {
        integration_slice_nr += 1;
        PROGRESS_MSG("starting timeslice " << start_time+integration_slice_nr*integration_time());

        // Check whether the integration slice continues past the stop time
        if (start_time+(integration_slice_nr+1)*integration_time() >
            stop_time) {
          status = STOP_CORRELATING;

          // Check whether the integration slice continues past the scan
        } else if (start_time+(integration_slice_nr+1)*integration_time() >
                   stop_time_scan) {
          // We can stop if we finished the last scan
          if (current_scan == control_parameters.number_scans()) {
            status = STOP_CORRELATING;
          } else {
            current_scan++;
            status = START_NEW_SCAN;
          }
        } else {
          // Just process the next time slice
          status = START_CORRELATION_TIME_SLICE;
        }
        break;
      }
      case STOP_CORRELATING: {
        // The status is set to END_NODE as soon as the output_node is ready
        MPI_Send(&output_slice_nr, 1, MPI_INT32,
                 RANK_OUTPUT_NODE, MPI_TAG_OUTPUT_NODE_CORRELATION_READY,
                 MPI_COMM_WORLD);

        status = WAIT_FOR_OUTPUT_NODE;
        break;
      }
      case WAIT_FOR_OUTPUT_NODE: {
        // The status is set to END_NODE as soon as the output_node is ready
        check_and_process_message();
        break;
      }
      case END_NODE: {
        break;
      }
    }
  }
  PROGRESS_MSG("terminating nodes");

  get_log_writer()(1) << "Terminating nodes" << std::endl;
}

void Manager_node::terminate()
{
	  PROGRESS_MSG("MESSAGE TERMINATE !");
		status = END_NODE;
}

void Manager_node::start_next_timeslice_on_node(int corr_node_nr) {
  int cross_channel = -1;
  if (control_parameters.cross_polarize()) {
    cross_channel = control_parameters.cross_channel(current_channel,
                    get_current_mode());
    SFXC_ASSERT((cross_channel == -1) || (cross_channel > (int)current_channel));
  }

  // Initialise the correlator node
  if (cross_channel == -1) {
    int32_t time = start_time + integration_time()*integration_slice_nr;
    get_log_writer()(1)
    << "start "
    << Vex::Date(start_year, start_day, time/1000).to_string()
    << (time%1000) << "ms"
    << ", channel " << current_channel << " to correlation node "
    << corr_node_nr << std::endl;
    PROGRESS_MSG("start "
                 << Vex::Date(start_year, start_day, time/1000).to_string()
                 << (time%1000) << "ms"
                 << ", channel " << current_channel << " to correlation node "
                 << corr_node_nr);
  } else {
    int32_t time = start_time + integration_time()*integration_slice_nr;
    get_log_writer()(1)
    << "start "
    << Vex::Date(start_year, start_day, time/1000).to_string()
    << (time%1000) << "ms"
    << ", channel "
    << current_channel << ","
    << cross_channel << " to correlation node "
    << corr_node_nr << std::endl;
    PROGRESS_MSG("start "
                 << Vex::Date(start_year, start_day, time/1000).to_string()
                 << (time%1000) << "ms"
                 << ", channel "
                 << current_channel << ","
                 << cross_channel << " to correlation node "
                 << corr_node_nr);
  }

  std::string channel_name =
    control_parameters.frequency_channel(current_channel);
  std::vector<std::string> station_name;
  Correlation_parameters correlation_parameters;
  int nr_stations = control_parameters.number_stations();
  for (int i=0; i<nr_stations; i++) {
    station_name.push_back(get_control_parameters().station(i));
  }
  correlation_parameters =
    control_parameters.
    get_correlation_parameters(control_parameters.scan(current_scan),
                               channel_name,
                               station_name,
                               get_input_node_map());
  correlation_parameters.start_time =
    start_time + integration_slice_nr*integration_time();
  correlation_parameters.stop_time  =
    start_time + (integration_slice_nr+1)*integration_time();
  correlation_parameters.integration_nr = integration_slice_nr;
  correlation_parameters.slice_nr = output_slice_nr;

  correlation_parameters.cross_polarize = (cross_channel != -1);

  // Check the cross polarisation
  if (cross_channel != -1) {
    int n_stations = control_parameters.number_stations();
    int n_streams = correlation_parameters.station_streams.size();
    SFXC_ASSERT(n_stations == n_streams);
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
  size_t nStations = control_parameters.number_stations();
  for (size_t station_nr=0;
       station_nr< nStations;
       station_nr++) {
    input_node_set_time_slice(control_parameters.station(station_nr),
                              current_channel,
                              /*stream*/corr_node_nr,
                              correlation_parameters.start_time,
                              correlation_parameters.stop_time);

    if (cross_channel != -1) {
      // Add the cross polarisation channel
      input_node_set_time_slice(control_parameters.station(station_nr),
                                cross_channel,
                                /*stream*/corr_node_nr+n_corr_nodes,
                                correlation_parameters.start_time,
                                correlation_parameters.stop_time);
    }
  }

  current_channel ++;
  current_correlator_node ++;
  if (control_parameters.cross_polarize()) {
    // Go to the next channel.
    cross_channel =
      control_parameters.cross_channel(current_channel,
                                       get_current_mode());
    while ((current_channel <
            control_parameters.number_frequency_channels()) &&
           (cross_channel >= 0) && (cross_channel < (int)current_channel)) {
      current_channel ++;
      cross_channel =
        control_parameters.cross_channel(current_channel,
                                         control_parameters.get_mode(start_time));
    }
  }
  output_slice_nr++;
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

  // Send the delay tables:
  get_log_writer() << "Set delay_table" << std::endl;
  for (size_t station=0;
       station < control_parameters.number_stations(); station++) {
    Delay_table_akima delay_table;
    const std::string &station_name = control_parameters.station(station);
    const std::string &delay_file =
      control_parameters.get_delay_table_name(station_name);
    delay_table.open(delay_file.c_str());
    if (!delay_table.initialised()) {
      DEBUG_MSG("Delay table could not be read");
      control_parameters.generate_delay_table(station_name, delay_file);
      delay_table.open(delay_file.c_str());
      char msg[120];
      snprintf(msg, 120,
               "Coudn't generate delay table, please remove '%s' and restart the correlator",
               delay_file.c_str());
      SFXC_ASSERT_MSG(delay_table.initialised(),
                      msg);
    }

    send(delay_table, /* station_nr */ 0, input_rank(station));
    correlator_node_set_all(delay_table, station_name);
  }

  Control_parameters::Date start = control_parameters.get_start_time();
  Control_parameters::Date stop = control_parameters.get_stop_time();
  start_year = start.year;
  start_day  = start.day;
  start_time = start.to_miliseconds();
  stop_time  = stop.to_miliseconds(start_day);

  // Get a list of all scan names
  current_scan = control_parameters.scan(start);
  SFXC_ASSERT(current_scan >= 0);
  SFXC_ASSERT((size_t)current_scan < control_parameters.number_scans());

  output_slice_nr = 0;

  PROGRESS_MSG("start_time: " << start.to_string());
  PROGRESS_MSG("stop_time: " << stop.to_string());

  get_log_writer()(1) << "Starting correlation" << std::endl;
}

void Manager_node::initialise_scan(const std::string &scan) {
  Vex::Date start_of_scan =
    control_parameters.get_vex().start_of_scan(scan);

  // set the start time to the beginning of the scan
  if (start_time + integration_slice_nr*integration_time() <
      start_of_scan.to_miliseconds(start_day)) {
    int64_t start_interval =
      start_of_scan.to_miliseconds(start_day)-start_time;
    integration_slice_nr = start_interval/integration_time();
    if ((start_interval%integration_time()) != 0) {
      integration_slice_nr ++;
    }
  }

  stop_time_scan =
    control_parameters.get_vex().stop_of_scan(scan).to_miliseconds(start_day);
  if (stop_time < stop_time_scan)
    stop_time_scan = stop_time;
  // Align the stop time with the time slices
  SFXC_ASSERT(((stop_time_scan-start_time)%integration_time()) >= 0);
  stop_time_scan -= (stop_time_scan-start_time)%integration_time();
  SFXC_ASSERT(((stop_time_scan-start_time)%integration_time()) == 0);


  // Send the track parameters to the input nodes
  const std::string &mode_name =
    control_parameters.get_vex().get_mode(scan);
  for (size_t station=0;
       station<control_parameters.number_stations(); station++) {
    const std::string &station_name =
      control_parameters.station(station);

    Input_node_parameters input_node_param =
      control_parameters.get_input_node_parameters(mode_name, station_name);
    input_node_set(station_name, input_node_param);
  }
}

void Manager_node::end_correlation() {
  SFXC_ASSERT(status == WAIT_FOR_OUTPUT_NODE);
  status = END_NODE;
}

std::string Manager_node::get_current_mode() const {
  std::string scan_name = control_parameters.scan(current_scan);
  return control_parameters.get_vex().get_mode(scan_name);
}

