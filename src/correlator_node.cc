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
#include "data_writer.h"
#include "utils.h"
#include "output_header.h"

Correlator_node::Correlator_node(int rank, int nr_corr_node)
    : Node(rank),
    correlator_node_ctrl(*this),
    data_readers_ctrl(*this),
    data_writer_ctrl(*this),
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



#ifdef RUNTIME_STATISTIC

  std::stringstream inputid;
  std::stringstream compid;
  std::stringstream monid;
  std::stringstream tt;

  inputid << "correlationnode" << RANK_OF_NODE;

  compid << inputid.str() << "_dotask";
  monid << compid.str() << "_monitor_state";
  dotask_state_.init(monid.str());
  dotask_state_.add_property(inputid.str(), "is_a", "correlationnode");
  dotask_state_.add_property(inputid.str(), "has", compid.str() );
  dotask_state_.add_property(compid.str(), "is_a", "correlationnode_dotaskloop");
  dotask_state_.add_property(compid.str(), "has", monid.str() );
  tt.str(monid.str());

  compid.str("");
  monid.str("");
  compid << inputid.str() << "_reader";
  monid << compid.str() << "_monitor_state";
  reader_state_.init(monid.str());
  reader_state_.add_property(inputid.str(), "is_a", "correlationnode");
  reader_state_.add_property(inputid.str(), "has", compid.str() );
  reader_state_.add_property(compid.str(), "is_a", "reader_component");
  reader_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );

  compid.str("");
  monid.str("");
  compid << inputid.str() << "_delaycorrection";
  monid << compid.str() << "_monitor_state";
  delaycorrection_state_.init(monid.str());
  delaycorrection_state_.add_property(inputid.str(), "is_a", "correlationnode");
  delaycorrection_state_.add_property(inputid.str(), "has", compid.str() );
  delaycorrection_state_.add_property(compid.str(), "is_a", "delaycorrection_component");
  delaycorrection_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );

  compid.str("");
  monid.str("");
  compid << inputid.str() << "_integration";
  monid << compid.str() << "_monitor_state";
  correlation_state_.init(monid.str());
  correlation_state_.add_property(inputid.str(), "is_a", "correlationnode");
  correlation_state_.add_property(inputid.str(), "has", compid.str() );
  correlation_state_.add_property(compid.str(), "is_a", "correlation_component");
  correlation_state_.add_property(compid.str(), "has", monid.str() );
  correlation_state_.add_property(tt.str(), "contains", monid.str() );
#endif //RUNTIME_STATISTIC

}

Correlator_node::~Correlator_node() {
#if PRINT_TIMER
  PROGRESS_MSG("Time bit_sample_reader:  " << bit_sample_reader_timer_.measured_time());
  PROGRESS_MSG("Time bits2float:  " << bits_to_float_timer_.measured_time());
  PROGRESS_MSG("Time delay:       " << delay_timer_.measured_time());
  PROGRESS_MSG("Time correlation: " << correlation_timer_.measured_time());
#endif
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

        if (correlation_core.almost_finished()) {
          if (n_integration_slice_in_time_slice==1) {
            // Notify manager node:
            int32_t msg = get_correlate_node_number();
            MPI_Send(&msg, 1, MPI_INT32, RANK_MANAGER_NODE,
                     MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED,
                     MPI_COMM_WORLD);
          }
        }
        if (correlation_core.finished()) {
          n_integration_slice_in_time_slice--;
          if (n_integration_slice_in_time_slice==0) {
            // Notify manager node:
            status = STOPPED;
            // Try initialising a new integration slice
            set_parameters();
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
  SFXC_ASSERT((size_t)sn < delay_modules.size());
  //  SFXC_ASSERT(integer_delay_modules[sn] != Integer_delay_correction_ptr());
  SFXC_ASSERT(delay_modules[sn] != Delay_correction_ptr());
  //  integer_delay_modules[sn]->set_delay_table(table);
  delay_modules[sn]->set_delay_table(table);
}

void Correlator_node::hook_added_data_reader(size_t stream_nr) {
  // create the bit sample reader tasklet
  if (bit_sample_readers.size() <= stream_nr) {
    bit_sample_readers.resize(stream_nr+1, Bit_sample_reader_ptr());
  }
  bit_sample_readers[stream_nr] =
    Bit_sample_reader_ptr(new Correlator_node_data_reader_tasklet());
  bit_sample_readers[stream_nr]->connect_to(data_readers_ctrl.get_data_reader(stream_nr));

  { // create the delay modules
    if (delay_modules.size() <= stream_nr) {
      delay_modules.resize(stream_nr+1,
                           boost::shared_ptr<Delay_correction>());
    }
    delay_modules[stream_nr] =
      Delay_correction_ptr(new Delay_correction());

    // Connect the delay_correction to the bits2float_converter
    delay_modules[stream_nr]->connect_to(bit_sample_readers[stream_nr]->get_output_buffer());
  }

  // Connect the correlation_core to delay_correction
  correlation_core.connect_to(stream_nr,
                              delay_modules[stream_nr]->get_output_buffer());
}

void Correlator_node::hook_added_data_writer(size_t i) {
  SFXC_ASSERT(i == 0);

  correlation_core.set_data_writer(data_writer_ctrl.get_data_writer(0));
}

int Correlator_node::get_correlate_node_number() {
  return nr_corr_node;
}

void Correlator_node::correlate() {
  RT_STAT( dotask_state_.begin_measure() );

  // Execute all tasklets:
  bit_sample_reader_timer_.resume();
  for (size_t i=0; i<bit_sample_readers.size(); i++) {
    SFXC_ASSERT(bit_sample_readers[i] != Bit_sample_reader_ptr());
    if (bit_sample_readers[i] != Bit_sample_reader_ptr()) {
      int count = 0;
      while ((count < 25) && bit_sample_readers[i]->has_work()) {
        RT_STAT( reader_state_.begin_measure() );

        bit_sample_readers[i]->do_task();
        RT_STAT( reader_state_.end_measure(1) );

        count++;
      }
    }
  }
  bit_sample_reader_timer_.stop();

  delay_timer_.resume();
  for (size_t i=0; i<delay_modules.size(); i++) {
    if (delay_modules[i] != Delay_correction_ptr()) {
      if (delay_modules[i]->has_work()) {
        RT_STAT( delaycorrection_state_.begin_measure() );

        delay_modules[i]->do_task();
        RT_STAT( delaycorrection_state_.end_measure(1) );

      }
    }
  }
  delay_timer_.stop();

  correlation_timer_.resume();
  if (correlation_core.has_work()) {
    RT_STAT( correlation_state_.begin_measure() );

    correlation_core.do_task();
    RT_STAT( correlation_state_.end_measure(1) );


  }

  correlation_timer_.stop();

  RT_STAT( dotask_state_.end_measure(1) );
}

void
Correlator_node::receive_parameters(const Correlation_parameters &parameters) {
  integration_slices_queue.push(parameters);

  { // Immediately start prefetching the data:
    int number_ffts_in_integration =
      Control_parameters::nr_ffts_per_integration_slice
      (parameters.integration_time,
       parameters.sample_rate,
       parameters.number_channels);
    for (size_t i=0; i<bit_sample_readers.size(); i++) {
      SFXC_ASSERT(bit_sample_readers[i] !=
             Bit_sample_reader_ptr());
      if (i <parameters.station_streams.size()) {
        bit_sample_readers[i]->set_parameters(number_ffts_in_integration,
                                              parameters.bits_per_sample,
                                              parameters.number_channels);
      }
    }
  }

  if (status == STOPPED)
    set_parameters();

}
void
Correlator_node::set_parameters() {
  SFXC_ASSERT(status == STOPPED);

  if (integration_slices_queue.empty())
    return;

  const Correlation_parameters &parameters =
    integration_slices_queue.front();

  int size_input_slice =
    Control_parameters::nr_bytes_per_integration_slice_input_node_to_correlator_node
    (parameters.integration_time,
     parameters.sample_rate,
     parameters.bits_per_sample,
     parameters.number_channels);

  SFXC_ASSERT(((parameters.stop_time-parameters.start_time) /
          parameters.integration_time) == 1);

  SFXC_ASSERT(size_input_slice > 0);

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
  int nBaselines = correlation_core.number_of_baselines();
  int size_of_one_baseline = sizeof(fftwf_complex)*
                             (parameters.number_channels*PADDING/2+1);

  output_node_set_timeslice(parameters.slice_nr,
                            parameters.slice_offset,
                            n_integration_slice_in_time_slice,
                            get_correlate_node_number(),
                            sizeof(Output_header_timeslice) +
                            ( nBaselines *
                              (size_of_one_baseline +
                               sizeof(Output_header_baseline) ) ) );

  integration_slices_queue.pop();
}

void
Correlator_node::
output_node_set_timeslice(int slice_nr, int slice_offset, int n_slices,
                          int stream_nr, int bytes) {
  correlation_core.data_writer()->set_size_dataslice(bytes*n_slices);
  int32_t msg_output_node[] = {stream_nr, slice_nr, bytes};
  for (int i=0; i<n_slices; i++) {
    MPI_Send(&msg_output_node, 3, MPI_INT32,
             RANK_OUTPUT_NODE,
             MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY,
             MPI_COMM_WORLD);
    msg_output_node[1] += slice_offset;
  }
}
