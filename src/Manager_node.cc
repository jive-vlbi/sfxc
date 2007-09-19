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

Manager_node::
Manager_node(int rank, int numtasks,
             Log_writer *log_writer,
             const Control_parameters &control_parameters)
  : Abstract_manager_node(rank, numtasks,
                          log_writer,
                          control_parameters),
    manager_controller(*this)
{
  assert(rank == RANK_MANAGER_NODE);

  add_controller(&manager_controller);

  get_log_writer()(0) << "Starting nodes" << std::endl;

  // initialise the log node
  start_log_node(RANK_LOG_NODE);
  start_output_node(RANK_OUTPUT_NODE);

  // Input nodes:
  int n_stations = get_control_parameters().number_stations();
  for (int input_node=0; input_node<n_stations; input_node++) {
    assert(input_node+3 != RANK_MANAGER_NODE);
    assert(input_node+3 != RANK_LOG_NODE);
    assert(input_node+3 != RANK_OUTPUT_NODE);
    assert(input_node+3 < numtasks);

    start_input_node(/*rank_nr*/ input_node+3);
  }
  assert(n_stations > 0);


  // correlator nodes:
  for (int correlator_rank=n_stations+3; 
       correlator_rank<numtasks; correlator_rank++) {
    assert(correlator_rank != RANK_MANAGER_NODE);
    assert(correlator_rank != RANK_LOG_NODE);
    assert(correlator_rank != RANK_OUTPUT_NODE);

    start_correlator_node(/*rank_nr*/ correlator_rank);
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
  get_log_writer()(0) << "Manager_node::start()" << std::endl;

  get_log_writer()(0) << "Initialising the Input_node" << std::endl;
  for (size_t station=0; 
       station<control_parameters.number_stations(); station++) {
    // setting the first data-source of the first station
    const std::string &station_name = control_parameters.station(station);
    std::string filename = control_parameters.data_sources(station_name)[0];
    set_single_data_reader(input_rank(station), filename);
  }

  // Get a list of all scan names
  std::vector<std::string> scans;
  control_parameters.get_vex().get_scans(std::back_inserter(scans));

  // Send the track parameters
  // PRECONDITION: they are the same for the entire experiment
  DEBUG_MSG("Set Track_parameters");
  get_log_writer() << "Set Track_parameters" << std::endl;
  for (size_t station=0; 
       station<control_parameters.number_stations(); station++) {
    const std::string &mode = 
      control_parameters.get_vex().get_mode(scans[0]);
    const std::string &station_name = 
      control_parameters.station(station);
    const std::string &track = 
      control_parameters.get_vex().get_track(mode, station_name);

    Track_parameters track_param =
      control_parameters.get_track_parameters(track);
    input_node_set(station, track_param);
  }

  // Send the delay tables:
  DEBUG_MSG("Set Delay_table");
  get_log_writer() << "Set Delay_table" << std::endl;
  for (size_t station=0; 
       station<control_parameters.number_stations(); station++) {
    Delay_table_akima delay_table;
    const std::string &station_name = control_parameters.station(station);
    const std::string &delay_file = 
      control_parameters.get_delay_table_name(station_name);
    delay_table.open(delay_file.c_str());

    correlator_node_set_all(delay_table);
  }
    
  DEBUG_MSG("Initialising data");
  int start_day  = control_parameters.get_start_time().day;
  int start_time = control_parameters.get_start_time().to_miliseconds();
  int64_t stop_time  = 
    control_parameters.get_stop_time().to_miliseconds(start_day);

  get_log_writer() << "start_day  : " << start_day << std::endl;
  get_log_writer() << "start_time : " << start_time << std::endl;
  get_log_writer() << "stop_time  : " << stop_time << std::endl;;

  size_t current_scan;

  {  // Iterate over all the scans to find the first scan to correlate
    const Vex &vex = control_parameters.get_vex();
  
    for (current_scan=0; 
         ((current_scan < scans.size()) &&
          (vex.stop_of_scan(scans[current_scan]).to_miliseconds(start_day) <= 
           start_time)); 
         current_scan++) {
      const std::string &mode = 
        control_parameters.get_vex().get_mode(scans[current_scan]);

      if (current_scan > 0) {
        // assume the same mode, hence the same track parameters
        assert(mode ==
               control_parameters.get_vex().get_mode(scans[current_scan-1]));
      }
    }
  }
  assert(current_scan < scans.size());
  get_log_writer() << "start scan : " << scans[current_scan] << std::endl;

  get_log_writer() << "Starting correlation" << std::endl;

  status = START_NEW_SCAN;
  
  // Do only one scan for now (for simplicity)
  while (status != END_NODE) {
    process_all_waiting_messages();
    DEBUG_MSG(status);
    switch (status) {
    case START_NEW_SCAN:
      {
        // Set the input nodes to the proper start time
        for (size_t station=0; station < control_parameters.number_stations();
             station++) {
          input_node_goto_time(station, start_time);
        }
        
        // Initialise the correlator nodes
        {
          Correlation_parameters correlation_parameters = 
            control_parameters.get_correlation_parameters(scans[current_scan]);
          correlator_node_set_all(correlation_parameters);
        }
        break;
      }
    case START_NEW_TIME_SLICE:
      {
        break;
      }
    case END_NODE:
      {
        break;
      }
    }
  }
  

//   // Set the output nodes
//   {
//     int channel_nr =0;
//     for (Track_parameters::Channel_iterator 
//            chan_it = track_param.channels.begin();
//          chan_it != track_param.channels.end(); chan_it++, channel_nr++) {
//       char filename[80];
//       snprintf(filename, 80, "file:///tmp/test_Input_node_output%d.out",
//                channel_nr);
//       set_multiple_data_writer(input_node, channel_nr, filename);
//     }
//     for (Track_parameters::Channel_iterator 
//            chan_it = track_param.channels.begin();
//          chan_it != track_param.channels.end(); chan_it++, channel_nr++) {
//       char filename[80];
//       snprintf(filename, 80, "file:///tmp/test_Input_node_output%d.out",
//                channel_nr);
//       set_multiple_data_writer(input_node, channel_nr, filename);
//     }
//   }

//   // Setting the start time
//   int64_t current_time = input_node_get_current_time(input_node);
//   int32_t start_time = control_parameters.get_start_time().to_miliseconds();
//   int32_t stop_time = start_time + 2000; // add two seconds
//   assert(current_time < start_time);
//   // goto the start time 
//   input_node_goto_time(input_node, start_time);
//   // set the stop time
//   input_node_set_stop_time(input_node, stop_time);

//   get_log_writer()(0) << "Writing a time slice" << std::endl;
//   if ( /* one_slice */ true ) {
//     int channel_nr =0;
//     for (Track_parameters::Channel_iterator 
//            chan_it = track_param.channels.begin();
//          chan_it != track_param.channels.end(); chan_it++, channel_nr++) {
//       input_node_set_time_slice(input_node, channel_nr, /*stream*/channel_nr,
//                                 start_time, stop_time);
//     }
//   } else {
//     int channel_nr =0;
//     for (Track_parameters::Channel_iterator 
//            chan_it = track_param.channels.begin();
//          chan_it != track_param.channels.end(); chan_it++, channel_nr++) {
//       input_node_set_time_slice(input_node, channel_nr, /*stream*/channel_nr,
//                                 start_time, start_time+1000);
//     }
//     for (Track_parameters::Channel_iterator 
//            chan_it = track_param.channels.begin();
//          chan_it != track_param.channels.end(); chan_it++, channel_nr++) {
//       input_node_set_time_slice(input_node, 
//                                 channel_nr-track_param.channels.size(),
//                                 /*stream*/channel_nr,
//                                 start_time+1000, stop_time);
//     }
//   }

//   // Waiting for the input node to finish
//   int status=get_status(input_node);
//   while (status != Input_node::WRITING) {
//     usleep(100000); // .1 second
//     status = get_status(input_node);
//   }
//   while (status == Input_node::WRITING) {
//     usleep(100000); // .1 second
//     status = get_status(input_node);
//   }
  
  get_log_writer()(0) << "Terminating nodes" << std::endl;
}



void
Manager_node::
set_correlating_state(size_t correlator_nr, Correlating_state state) {
  assert(correlator_nr < state_correlator_node.size());
  state_correlator_node[correlator_nr] = state;
}











// Manager_node::Manager_node(int numtasks, int rank, char * control_file) 
//   : Node(rank), numtasks(numtasks), slicenr(0), 
//     manager_controller(*this)
// {
//   get_log_writer() << "Manager_node()" << std::endl;
//   MPI_Status status;
  
//   assert(rank == 0);

//   add_controller(&manager_controller);

//   { // Initialise the log node, otherwise no error messages can be sent.
//     assert (RANK_LOG_NODE == 1);
    
//     int msg=0;
//     // Log node:
//     MPI_Send(&msg, 1, MPI_INT32, 
//              RANK_LOG_NODE, MPI_TAG_SET_LOG_NODE, MPI_COMM_WORLD);
//     MPI_Send(&msg, 1, MPI_INT32, 
//              RANK_LOG_NODE, MPI_TAG_LOG_NODE_SET_OUTPUT_COUT, MPI_COMM_WORLD);
//     MPI_Recv(&msg, 1, MPI_INT32, 
//              RANK_LOG_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
//   }  

  
//   int err;
//   err = read_control_file(control_file);
//   if (err != 0) return;

//   START_INPUT_NODES = 3;
// #ifdef READ_DATA_FOR_CORRELATE_NODES_FROM_FILE
//   // Reading from file does not need input nodes:
//   N_INPUT_NODES = 0;
// #else
//   N_INPUT_NODES = GenPrms.get_nstations();
//   assert(N_INPUT_NODES > 0);
// #endif
  
//   START_CORRELATE_NODES = START_INPUT_NODES + N_INPUT_NODES;
//   N_CORRELATE_NODES = numtasks - START_CORRELATE_NODES;

//   assert(N_CORRELATE_NODES >= 1);

//   { // Initialise the output node
//     assert (RANK_OUTPUT_NODE == 2);
    
//     int msg=0;
//     const char *filename = GenPrms.get_corfile();
//     MPI_Send(&msg, 1, MPI_INT32, 
//              RANK_OUTPUT_NODE, MPI_TAG_SET_OUTPUT_NODE, MPI_COMM_WORLD);
//     MPI_Send((void *)filename, strlen(filename)+1, MPI_CHAR, 
//              RANK_OUTPUT_NODE, MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);

//     int64_t channel;
//     MPI_Recv(&channel, 1, MPI_INT64, MPI_ANY_SOURCE,
//              MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
//     assert((int)status.MPI_SOURCE == RANK_OUTPUT_NODE);               

//     MPI_Recv(&msg, 1, MPI_INT32, 
//              RANK_OUTPUT_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
//   }  

//   // set all nodes to INITIALISING:
//   state_correlate_nodes.resize(numtasks, INITIALISING);

//   // Nstations+3 and later are correlate nodes
//   {
//     int n_nodes_initialised=0;
//     bool connection[numtasks];
//     for (int i=0; i<numtasks; i++) connection[i] = false;

//     for (int i=0; i<N_CORRELATE_NODES; i++) {
//       // starting a correlator node
//       MPI_Send(&i, 1, MPI_INT32, i+START_CORRELATE_NODES, 
//                MPI_TAG_SET_CORRELATOR_NODE, MPI_COMM_WORLD);
//       int msg;
//       MPI_Recv(&msg, 1, MPI_INT32, 
//                i+START_CORRELATE_NODES, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
  
//       err = send_control_parameters_to_controller_node(i+START_CORRELATE_NODES);
//       if (err != 0) return;
  
      
//       // Set the output node:
// #ifdef USE_FILES_FOR_OUTPUT                
//       char output_filename[80];
//       sprintf(output_filename, "%s.%d", GenPrms.get_corfile(), i);
//       MPI_Send(output_filename, strlen(output_filename)+1, MPI_CHAR,
//                i+START_CORRELATE_NODES, MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);
// #else
//       int32_t ranks[2] = {i, RANK_OUTPUT_NODE};
//       MPI_Send(ranks, 2, MPI_INT32, 
//                i+START_CORRELATE_NODES, 
//                MPI_TAG_SET_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP, MPI_COMM_WORLD);
// #endif

//       int result;
//       MPI_Status stat;
//       MPI_Iprobe(MPI_ANY_SOURCE, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &result, &stat);
//       while (result) { // Wait until all connections are set up properly
//         int64_t channel;
//         MPI_Recv(&channel, 1, MPI_INT64, MPI_ANY_SOURCE,
//                  MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &stat);
//         assert((int)channel < N_CORRELATE_NODES);               
//         assert(!connection[channel]);
//         connection[channel] = true;
//         n_nodes_initialised++;

//         MPI_Iprobe(MPI_ANY_SOURCE, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &result, &stat);
//       }
//     }
//     while (n_nodes_initialised < N_CORRELATE_NODES) {
//       MPI_Status stat;
//       int64_t channel;
//       MPI_Recv(&channel, 1, MPI_INT64, MPI_ANY_SOURCE,
//                MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &stat);
//       assert(!connection[channel]);
//       connection[channel] = true;
//       n_nodes_initialised++;
//     }
//   }

//   // Initialise input nodes
//   for (int i=0; i<N_INPUT_NODES; i++) {
// #ifndef READ_DATA_FOR_CORRELATE_NODES_FROM_FILE
//     // starting an input reader
//     MPI_Send(&i, 1, MPI_INT32, 
//              i+START_INPUT_NODES, MPI_TAG_SET_INPUT_NODE, MPI_COMM_WORLD);

//     // Send the necessary control parameters:
//     MPI_Transfer mpi_transfer;
//     mpi_transfer.send_general_parameters(i+START_INPUT_NODES, RunPrms, GenPrms, StaPrms);

//     int msg;
//     MPI_Recv(&msg, 1, MPI_INT32, 
//              i+START_INPUT_NODES, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD,
//              &status);

//     char *filename = StaPrms[i].get_mk4file();
//     // strlen+1 so that \0 gets transmitted as well
//     MPI_Send(filename, strlen(filename)+1, MPI_CHAR, 
//              i+START_INPUT_NODES, MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);

//     { // Wait for the connection to be established
//       int64_t msg;
//       MPI_Recv(&msg, 1, MPI_INT64, MPI_ANY_SOURCE,
//                MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
//       assert(status.MPI_SOURCE == i+START_INPUT_NODES);
//     }
// #endif

//     // Add correlator nodes to the input readers:
//     for (int j=0; j<N_CORRELATE_NODES; j++) {

// #ifdef READ_DATA_FOR_CORRELATE_NODES_FROM_FILE
//       char filename[strlen(StaPrms[i].get_mk4file())+1];
//       sprintf(filename, "%c%s", (char)i, StaPrms[i].get_mk4file());
//       // strlen+1 so that \0 gets transmitted as well
//       MPI_Send(filename, strlen(filename+1)+2, MPI_CHAR, 
//                j+START_CORRELATE_NODES, MPI_TAG_ADD_DATA_READER_FILE, MPI_COMM_WORLD);
//       // Wait until the connection is set up:
//       int64_t msg;
//       MPI_Recv(&msg, 1, MPI_INT64, MPI_ANY_SOURCE,
//                MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
//       assert(status.MPI_SOURCE == j+START_CORRELATE_NODES);               
// #else
//       // Writer_stream_nr, reader_stream_nr, output_node
//       int32_t ranks[3] = {j, i, j+START_CORRELATE_NODES};
//       MPI_Send(ranks, 3, MPI_INT32, 
//                i + START_INPUT_NODES,
//                MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP, MPI_COMM_WORLD);
//       // Wait until the connection is set up:
//       int64_t msg;
//       MPI_Recv(&msg, 1, MPI_INT64, MPI_ANY_SOURCE,
//                MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
//       assert(status.MPI_SOURCE == i + START_INPUT_NODES);               
// #endif
//     }
//   }

//   if (err != 0) return;
  
//   // set manager_controller:
//   get_log_writer()(1) << "MPI: Initialisation ready" << std::endl;
// }

// void Manager_node::start() {
// #ifndef READ_DATA_FOR_CORRELATE_NODES_FROM_FILE
//   {
//     for (int i=0; i<N_INPUT_NODES; i++) {
//       assert(GenPrms.get_usStart() >= get_start_time(i));
//       goto_start_time(i, GenPrms.get_usStart());
//       assert(GenPrms.get_usStart() == get_start_time(i));
//     }
//   }
// #endif
  
  
//   int last_correlator_node = -1;
//   while (GenPrms.get_duration() > 0) {
//     // Check for MPI messages
//     int result;
//     while ((result = check_and_process_waiting_message()) != NO_MESSAGE) {
//       if (result == TERMINATE_NODE) {
//         return;
//       }
//     }
    
//     // Check if there are Correlate nodes waiting:
//     bool searching = true;
    
//     // Duration of one time slice in seconds:
//     int slice_duration = 1;
//     if (N_CORRELATE_NODES == 1) {
//       get_log_writer()(0) << "Only one correlate node, only one time slice"
//                           << std::endl;
//       slice_duration = GenPrms.get_duration();
//     }

//     for (int i=0; (i<numtasks) && (GenPrms.get_duration()>0); i++) {
//       if ((state_correlate_nodes[i] == READY) && (last_correlator_node != i)) {
//         searching = false;
//         last_correlator_node = i;

//         get_log_writer()(0) << "Time slice: " << slicenr 
//                             << " to " << i-START_CORRELATE_NODES << std::endl;
//         // Initialise the slice on the correlator node
//         int64_t times[] = {slicenr,
//                            GenPrms.get_usStart(),
//                            std::min(GenPrms.get_duration(), slice_duration)};
//         MPI_Send(times, 3, MPI_INT64, last_correlator_node,
//                  MPI_TAG_CORRELATE_TIME_SLICE, MPI_COMM_WORLD);

//         // Initialise the slice on the input node
//         int64_t priority_in[] = {last_correlator_node-START_CORRELATE_NODES, 
//                                  slicenr, 
//                                  times[1],
//                                  times[1]+times[2]*1000000};

//         for (int input_node = 0; input_node < N_INPUT_NODES; input_node++) {
//           MPI_Send(&priority_in, 4, MPI_INT64, 
//                    START_INPUT_NODES+input_node, 
//                    MPI_TAG_INPUT_NODE_INPUT_STREAM_SET_PRIORITY, 
//                    MPI_COMM_WORLD);
//         }

//         // Initialise the slice on the output node
//         int nstations = GenPrms.get_nstations();
//         int size_of_baseline = 
//           sizeof(fftw_complex)*(GenPrms.get_n2fft()*GenPrms.get_pad()/2+1);
//         int integration_steps = (int)(slice_duration/GenPrms.get_time2avg());
//         int nr_baselines = -1;
//         if (0 <= RunPrms.get_ref_station(0) && 
//             RunPrms.get_ref_station(0) < nstations) {
//           //use a reference station
//           if (0 <= RunPrms.get_ref_station(1) && 
//               RunPrms.get_ref_station(1) < nstations) {
//             nr_baselines    = 3*nstations-2;
//           } else {
//             nr_baselines    = 2*nstations-1;
//           }
//         } else {
//           //correlate all baselines
//           nr_baselines    = nstations*(nstations-1)/2 + nstations;
//         }
        
//         int64_t msg_output_node[] =
//           {last_correlator_node-START_CORRELATE_NODES, 
//            slicenr,
//            integration_steps * nr_baselines * size_of_baseline};
//         MPI_Send(&msg_output_node, 3, MPI_INT64,
//                  RANK_OUTPUT_NODE,
//                  MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY,
//                  MPI_COMM_WORLD);


//         // Only one time slice for now
//         GenPrms.set_duration(GenPrms.get_duration()-times[2]);
//         GenPrms.set_usStart(GenPrms.get_usStart()+times[2]*1000000);

//         slicenr ++;

//         // The node is now correlating:
//         state_correlate_nodes[i] = CORRELATING;
//       }
//     }
    
//     if (searching) {
//       // Nothing will happen until the next MPI message arrives: 
//       check_and_process_message();
//     }    
//   }
  


//   // Close connections: First receive all waiting messages
//   int result;
//   while ((result = check_and_process_waiting_message()) != NO_MESSAGE) {
//     if (result == TERMINATE_NODE) {
//       return;
//     }
//   }

//   // Wait untill all correlation nodes are finished
//   bool finished;
//   do {
//     finished = true; 
//     for (int i=0; (i<numtasks); i++) {
//       finished &= (state_correlate_nodes[i] != CORRELATING);
//     }
//     if (!finished) {
//       // Nothing will happen until the next MPI message arrives: 
//       check_and_process_message();
//     }
//   } while (!finished);

//   // Terminate the correlate node
//   for (int i=0; i<N_CORRELATE_NODES; i++) {
//     int type = 0;
//     MPI_Send(&type, 1, MPI_INT32, 
//              START_CORRELATE_NODES+i, MPI_TAG_END_NODE, MPI_COMM_WORLD);
//   }
  
//   // Terminate the output node  
//   MPI_Send(&slicenr, 1, MPI_INT32, RANK_OUTPUT_NODE, 
//            MPI_TAG_OUTPUT_NODE_CORRELATION_READY, MPI_COMM_WORLD);
  
//   // Terminate the correlator nodes
//   for (int i=0; i<N_INPUT_NODES; i++) {
//     int64_t stop_time = GenPrms.get_usStart() + GenPrms.get_usDur();
//     MPI_Send(&stop_time, 1, MPI_INT64, 
//              START_INPUT_NODES+i, MPI_TAG_INPUT_NODE_STOP_TIME, MPI_COMM_WORLD);
//   }
//   // Terminate the input nodes
//   int type = 0;
//   for (int i=0; i<N_INPUT_NODES; i++) {
//     MPI_Send(&type, 1, MPI_INT32, i+START_INPUT_NODES, 
//              MPI_TAG_END_NODE, MPI_COMM_WORLD);
//   }

// }

// int Manager_node::read_control_file(char *control_file) {
//   if (initialise_control(control_file, get_log_writer(),
//                          RunPrms, GenPrms, StaPrms) != 0) {
//     get_log_writer()(0) << "Error: Initialisation using control file failed"
//                         << std::endl;;
//     return 1;
//   }
//   return 0;
// }

// int Manager_node::send_control_parameters_to_controller_node(int node) {
//   // Send the necessary control parameters:
//   MPI_Transfer mpi_transfer;
//   mpi_transfer.send_general_parameters(node, RunPrms, GenPrms, StaPrms);

//   // Send the delay tables:
//   for (int sn=0; sn<GenPrms.get_nstations(); sn++) {
//     Delay_table_akima delay; 
//     get_log_writer() << StaPrms[sn].get_delaytable() << std::endl;
//     delay.set_cmr(GenPrms);
//     int retval = delay.open(StaPrms[sn].get_delaytable());
//     if (retval != 0) {
//       get_log_writer()(0) << "Error while reading delay table."
//                           << std::endl;;
//       return retval;
//     }
//     mpi_transfer.send(delay, sn, node);
//   }

//   return 0;
// }


// void Manager_node::hook_added_data_reader(size_t reader) {
// }

// void Manager_node::hook_added_data_writer(size_t writer) {
// }

// void Manager_node::set_correlating_state(int node, CORRELATING_STATE state) {
//   assert ((node >= 0) && (((size_t)node) < state_correlate_nodes.size()));
//   state_correlate_nodes[node] = state;
// }

// int64_t Manager_node::get_start_time(int input_node_nr) {
//   assert(input_node_nr >= 0);
//   assert(input_node_nr < N_INPUT_NODES);
//   int64_t time;
//   // Get initial start time
//   MPI_Send(&time, 1, MPI_INT64, 
//            START_INPUT_NODES+input_node_nr, MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP, 
//            MPI_COMM_WORLD);
//   MPI_Status status;           
//   MPI_Recv(&time, 1, MPI_INT64, 
//            START_INPUT_NODES+input_node_nr, MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP, 
//            MPI_COMM_WORLD, &status);
//   return time;
// }

// void Manager_node::goto_start_time(int input_node_nr, int64_t time) {
//   assert(input_node_nr >= 0);
//   assert(input_node_nr < N_INPUT_NODES);
//   MPI_Send(&time, 1, MPI_INT64, 
//            START_INPUT_NODES+input_node_nr, MPI_TAG_INPUT_NODE_GOTO_TIME, 
//            MPI_COMM_WORLD);
// }

