/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "Correlator_node.h"

#include "Data_reader_buffer.h"
#include "Data_writer_buffer.h"
#include "utils.h"

Correlator_node::Correlator_node(int rank, int nr_corr_node)
 : Node(rank),
   correlator_node_ctrl(*this),
   data_readers_ctrl(*this),
   data_writer_ctrl(*this),
   correlate_state(INITIALISE_TIME_SLICE),
   status(STOPPED),
   nr_corr_node(nr_corr_node)
{
  DEBUG_MSG(__PRETTY_FUNCTION__);
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
  DEBUG_MSG("/" << __PRETTY_FUNCTION__);
}

Correlator_node::~Correlator_node()
{
}

void Correlator_node::start()
{
  DEBUG_MSG(__PRETTY_FUNCTION__);
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
        get_log_writer()(2) << " status = CORRELATING" << std::endl;
        if (process_all_waiting_messages() == TERMINATE_NODE) {
          status = END_CORRELATING;
        }
        
        correlate();

        if (correlation_core.finished()) {
          n_integration_slice_in_time_slice--;
          if (n_integration_slice_in_time_slice==0) {
            DEBUG_MSG("Finished timeslice");
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
  DEBUG_MSG("/" << __PRETTY_FUNCTION__);
}


void Correlator_node::add_delay_table(int sn, Delay_table_akima &table) {
  assert((size_t)sn < delay_modules.size());
  assert(integer_delay_modules[sn] != Integer_delay_correction_ptr());
  assert(delay_modules[sn] != Delay_correction_ptr());
  DEBUG_MSG("Setting delay table for " << sn)
  integer_delay_modules[sn]->set_delay_table(table);
  delay_modules[sn]->set_delay_table(table);
}

void Correlator_node::hook_added_data_reader(size_t stream_nr) {
  DEBUG_MSG("added input stream: " << stream_nr);
  // NGHK: TODO: Make sure a time slice fits
  
  boost::shared_ptr< Input_buffer > buffer(new Input_buffer(25));
  data_readers_ctrl.set_buffer(stream_nr, buffer);

  boost::shared_ptr<Bits_to_float_converter> sample_reader(new Bits_to_float_converter());
  sample_reader->set_data_reader(data_readers_ctrl.get_data_reader(stream_nr));

  // create the bits to float converters
  if (bits2float_converters.size() <= stream_nr) {
    bits2float_converters.resize(stream_nr+1, 
                                 boost::shared_ptr<Bits_to_float_converter>());
  }
  bits2float_converters[stream_nr] = sample_reader;

  { // create the integer delay modules
    if (integer_delay_modules.size() <= stream_nr) {
      integer_delay_modules.resize(stream_nr+1, 
                                   Integer_delay_correction_ptr());
    }
    integer_delay_modules[stream_nr] = 
      Integer_delay_correction_ptr(new Integer_delay_correction());
    if (stream_nr == 1) {
      integer_delay_modules[stream_nr]->verbose = true;
    }

    // Connect the delay_correction to the bits2float_converter
    integer_delay_modules[stream_nr]->connect_to(sample_reader->get_output_buffer());
  }

  { // create the delay modules
    if (delay_modules.size() <= stream_nr) {
      delay_modules.resize(stream_nr+1, 
                           boost::shared_ptr<Delay_correction>());
    }
    delay_modules[stream_nr] = 
      Delay_correction_ptr(new Delay_correction());
    if (stream_nr == 1) {
      delay_modules[stream_nr]->verbose = true;
    }

    // Connect the delay_correction to the bits2float_converter
    delay_modules[stream_nr]->connect_to(integer_delay_modules[stream_nr]->get_output_buffer());
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
  for (size_t i=0; i<bits2float_converters.size(); i++) {
    if (bits2float_converters[i] != Bits2float_ptr()) {
      bits2float_converters[i]->do_task();
    }
  }
  for (size_t i=0; i<integer_delay_modules.size(); i++) {
    if (integer_delay_modules[i] != Integer_delay_correction_ptr()) {
      integer_delay_modules[i]->do_task();
    }
  }
  for (size_t i=0; i<delay_modules.size(); i++) {
    if (delay_modules[i] != Delay_correction_ptr()) {
      delay_modules[i]->do_task();
    }
  }
  correlation_core.do_task();
}

void Correlator_node::set_parameters(const Correlation_parameters &parameters) {
  assert(status == STOPPED);
  DEBUG_MSG(__PRETTY_FUNCTION__);

  for (size_t i=0; i<bits2float_converters.size(); i++) {
    if (bits2float_converters[i] != Bits2float_ptr()) {
      DEBUG_MSG("Bit2float: Setting parameters for stream " << i);
      bits2float_converters[i]->set_parameters(parameters.bits_per_sample,
                                               parameters.number_channels);
    }
  }
  for (size_t i=0; i<integer_delay_modules.size(); i++) {
    if (integer_delay_modules[i] != Integer_delay_correction_ptr()) {
      DEBUG_MSG("Integer delay: Setting parameters for stream " << i);
      integer_delay_modules[i]->set_parameters(parameters);
    }
  }
  for (size_t i=0; i<delay_modules.size(); i++) {
    if (delay_modules[i] != Delay_correction_ptr()) {
      DEBUG_MSG("Delay: Setting parameters for stream " << i);
      delay_modules[i]->set_parameters(parameters);
    }
  }
  correlation_core.set_parameters(parameters);

  
  status = CORRELATING;
  
  n_integration_slice_in_time_slice = 
    (parameters.stop_time-parameters.start_time) / parameters.integration_time;
  
  DEBUG_MSG(n_integration_slice_in_time_slice);
  DEBUG_MSG("/" << __PRETTY_FUNCTION__);
}
