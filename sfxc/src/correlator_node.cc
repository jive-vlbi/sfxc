/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "correlator_node.h"

#include "data_reader_buffer.h"
#include "data_writer_buffer.h"
#include "utils.h"
#include "output_header.h"

Correlator_node::Correlator_node(int rank, int nr_corr_node)
    : Node(rank),
    correlator_node_ctrl(*this),
    data_readers_ctrl(*this),
    data_writer_ctrl(*this),
    correlate_state(INITIALISE_TIME_SLICE),
    status(STOPPED),
nr_corr_node(nr_corr_node) {
  get_log_writer()(1) << "Correlator_node(" << nr_corr_node << ")" << std::endl;

  add_controller(&correlator_node_ctrl);
  add_controller(&data_readers_ctrl);
  add_controller(&data_writer_ctrl);


  int32_t msg;
  MPI_Send(&msg, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);

  MPI_Send(&nr_corr_node, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED,
           MPI_COMM_WORLD);
}

Correlator_node::~Correlator_node() {
  DEBUG_MSG("Time bits2float:  " << bits_to_float_timer_.measured_time());
  DEBUG_MSG("Time delay:       " << delay_timer_.measured_time());
  DEBUG_MSG("Time correlation: " << correlation_timer_.measured_time());
}

void Correlator_node::start() {
  while (true) {
    switch (status) {
      case STOPPED: {
        // blocking:
        if (check_and_process_message()==TERMINATE_NODE) {
          status = END_CORRELATING;
        }
        break;
      }
      case CORRELATING: {
        if (process_all_waiting_messages() == TERMINATE_NODE) {
          status = END_CORRELATING;
        }

        correlate();

        if (correlation_core.finished()) {
          n_integration_slice_in_time_slice--;
          if (n_integration_slice_in_time_slice==0) {
            // Notify manager node:
            int32_t msg = get_correlate_node_number();
            MPI_Send(&msg, 1, MPI_INT32, RANK_MANAGER_NODE,
                     MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED,
                     MPI_COMM_WORLD);
            status = STOPPED;
          }
        }
        break;
      }
      case END_CORRELATING: {
        return;
      }
    }
  }
}


void Correlator_node::add_delay_table(int sn, Delay_table_akima &table) {
  assert((size_t)sn < delay_modules.size());
  //  assert(integer_delay_modules[sn] != Integer_delay_correction_ptr());
  assert(delay_modules[sn] != Delay_correction_ptr());
  //  integer_delay_modules[sn]->set_delay_table(table);
  delay_modules[sn]->set_delay_table(table);
}

void Correlator_node::hook_added_data_reader(size_t stream_nr) {
  // Make sure a time slice fits, at most 16000 ffts of length 1024 samples
  Input_buffer_element elem;
  Input_buffer_ptr buffer(new Input_buffer(16000*(1024/4)/elem.size()));
  data_readers_ctrl.set_buffer(stream_nr, buffer);

  boost::shared_ptr<Bits_to_float_converter> sample_reader(new Bits_to_float_converter());
  sample_reader->set_data_reader(data_readers_ctrl.get_data_reader(stream_nr));

  // create the bits to float converters
  if (bits2float_converters.size() <= stream_nr) {
    bits2float_converters.resize(stream_nr+1,
                                 boost::shared_ptr<Bits_to_float_converter>());
  }
  bits2float_converters[stream_nr] = sample_reader;

  { // create the delay modules
    if (delay_modules.size() <= stream_nr) {
      delay_modules.resize(stream_nr+1,
                           boost::shared_ptr<Delay_correction>());
    }
    delay_modules[stream_nr] =
      Delay_correction_ptr(new Delay_correction());

    // Connect the delay_correction to the bits2float_converter
    delay_modules[stream_nr]->connect_to(bits2float_converters[stream_nr]->get_output_buffer());
  }

  // Connect the correlation_core to delay_correction
  correlation_core.connect_to(stream_nr,
                              delay_modules[stream_nr]->get_output_buffer());
}

void Correlator_node::hook_added_data_writer(size_t i) {
  assert(i == 0);

  correlation_core.set_data_writer(data_writer_ctrl.get_data_writer(0));
}

int Correlator_node::get_correlate_node_number() {
  return nr_corr_node;
}

/** Number of integration steps done in the current time slice **/
int Correlator_node::number_of_integration_steps_in_time_slice() {
  assert(false);
  return 0;
}

/** Size in bytes of the output of one integration step **/
int Correlator_node::output_size_of_one_integration_step() {
  assert(false);
  return 0;
}

void Correlator_node::correlate() {
  // Execute all tasklets:
  bits_to_float_timer_.resume();
  for (size_t i=0; i<bits2float_converters.size(); i++) {
    if (bits2float_converters[i] != Bits2float_ptr()) {
      if (bits2float_converters[i]->has_work())
        bits2float_converters[i]->do_task();
    }
  }
  bits_to_float_timer_.stop();
  
  delay_timer_.resume();
  for (size_t i=0; i<delay_modules.size(); i++) {
    if (delay_modules[i] != Delay_correction_ptr()) {
      delay_modules[i]->do_task();
    }
  }
  delay_timer_.stop();
  
  correlation_timer_.resume();
  correlation_core.do_task();
  correlation_timer_.stop();
}

void
Correlator_node::set_parameters(const Correlation_parameters &parameters) {
  assert(status == STOPPED);

  int size_input_slice =
    Control_parameters::nr_bytes_per_integration_slice_input_node_to_correlator_node
    (parameters.integration_time,
     parameters.sample_rate,
     parameters.bits_per_sample,
     parameters.number_channels);

  int nr_integrations =
    (parameters.stop_time-parameters.start_time)/parameters.integration_time;

  assert(size_input_slice > 0);

  for (size_t i=0; i<bits2float_converters.size(); i++) {
    if (bits2float_converters[i] != Bits2float_ptr()) {
      bits2float_converters[i]->set_parameters(parameters.bits_per_sample,
          size_input_slice*nr_integrations,
          parameters.number_channels);
    }
  }
  for (size_t i=0; i<delay_modules.size(); i++) {
    if (delay_modules[i] != Delay_correction_ptr()) {
      delay_modules[i]->set_parameters(parameters);
    }
  }
  correlation_core.set_parameters(parameters, get_correlate_node_number());


  status = CORRELATING;

  n_integration_slice_in_time_slice =
    (parameters.stop_time-parameters.start_time) / parameters.integration_time;
  // set the output stream
   int nAutos = parameters.station_streams.size();
   int nCrosses = nAutos*(nAutos-1)/2;
   int nBaselines;
   if (parameters.cross_polarize) { // do cross polarisation
     if (parameters.reference_station < 0) {
       nBaselines = 2*nAutos + 4*nCrosses;
     } else {
       nBaselines = 2*nAutos + 4*(nAutos-1);
     }
   } else {
     if (parameters.reference_station < 0) {
       nBaselines = nAutos + nCrosses;
     } else {
       nBaselines = 2*nAutos - 1;
     }
   }
        
   int size_of_one_baseline = sizeof(fftwf_complex)*
     (parameters.number_channels*PADDING/2+1);
   
   output_node_set_timeslice(parameters.slice_nr, get_correlate_node_number(),
         n_integration_slice_in_time_slice * size_of_one_baseline*nBaselines +
         n_integration_slice_in_time_slice * sizeof(Output_header_timeslice) +
         n_integration_slice_in_time_slice * sizeof(Output_header_baseline)*nBaselines);
 }

 void
 Correlator_node::
 output_node_set_timeslice(int slice_nr, int stream_nr, int bytes) {
   int32_t msg_output_node[] = {stream_nr, slice_nr, bytes};
   MPI_Send(&msg_output_node, 3, MPI_INT32,
            RANK_OUTPUT_NODE,
            MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY,
            MPI_COMM_WORLD);
 }
