/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Aard Keimpema <Keimpema@JIVE.nl>, 2008
 *
 * $Id$
 *
 */

#include "correlator_node.h"
#include "correlation_core_phased.h"
#include "data_reader_buffer.h"
#include "data_writer.h"
#include "utils.h"
#include "output_header.h"
#include "delay_correction.h"
#ifdef USE_IPP
#include <ippcore.h>
#endif

Correlator_node::Correlator_node(int rank, int nr_corr_node, bool pulsar_binning_, bool phased_array_)
    : Node(rank),
    correlator_node_ctrl(*this),
    data_readers_ctrl(*this),
    data_writer_ctrl(*this),
    status(STOPPED),
    isinitialized_(false),
    nr_corr_node(nr_corr_node), 
    pulsar_parameters(get_log_writer()),
    pulsar_binning(pulsar_binning_),
    phased_array(phased_array_), n_streams(0) {
  #ifdef USE_IPP
  ippSetNumThreads(1);
  #endif
  get_log_writer()(1) << "Correlator_node(" << nr_corr_node << ")" << std::endl;
  if (phased_array){
    correlation_core_normal = new Correlation_core_phased();
    correlation_core = correlation_core_normal;
  }else{
    correlation_core_normal = new Correlation_core();
    if(pulsar_binning){
      correlation_core_pulsar = new Correlation_core_pulsar();
      correlation_core = correlation_core_pulsar;
    }else
      correlation_core = correlation_core_normal;
  }
  add_controller(&correlator_node_ctrl);
  add_controller(&data_readers_ctrl);
  add_controller(&data_writer_ctrl);


  int32_t msg;
  MPI_Send(&msg, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);

  MPI_Send(&nr_corr_node, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED,
           MPI_COMM_WORLD);

  has_requested=true;


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

void Correlator_node::start_threads() {
  threadpool_.register_thread( reader_thread_.start() );
  threadpool_.register_thread( bit2float_thread_.start() );
}

void Correlator_node::stop_threads() {
  reader_thread_.stop();
  bit2float_thread_.stop();

  /// We wait the termination of the threads
  threadpool_.wait_for_all_termination();
}

void Correlator_node::start() {
  /// We enter the main loop of the coorelator node.
  //DEBUG_MSG("START MAIN LOOp !");
  main_loop();
}

void Correlator_node::terminate() {
  DEBUG_MSG("Correlator node received terminate signal.");
  status = END_CORRELATING;
}

void Correlator_node::main_loop() {
  while ( status != END_CORRELATING ) {
    switch (status) {
    case STOPPED: {
        /// We wait for a message
        check_and_process_message();
        // blocking:
        break;
      }
    case CORRELATING: {
        process_all_waiting_messages();

        correlate();
        if (!has_requested && correlation_core->almost_finished()) {
          int32_t msg = get_correlate_node_number();
          MPI_Send(&msg, 1, MPI_INT32, RANK_MANAGER_NODE,
                   MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED,
                   MPI_COMM_WORLD);

          has_requested = true;
        }
        if (correlation_core->finished()) {
          ///DEBUG_MSG("CORRELATION CORE FINISHED !" << n_integration_slice_in_time_slice);
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
    case END_CORRELATING:
      break;
    }
  }
  stop_threads();
}

void Correlator_node::add_delay_table(Delay_table &table, int sn1, int sn2) {
  SFXC_ASSERT(sn1<=sn2);
  SFXC_ASSERT((size_t)sn1 < delay_modules.size());
  SFXC_ASSERT((size_t)sn2 < delay_modules.size());
  SFXC_ASSERT(delay_modules[sn1] != Delay_correction_ptr());
  SFXC_ASSERT(delay_modules[sn2] != Delay_correction_ptr());
  if(delay_tables.size() <= sn1)
    delay_tables.resize(sn1+1);
  if(delay_index.size() <= sn2)
    delay_index.resize(sn2+1, -1);

  delay_tables[sn1].add_scans(table);
  delay_index[sn1] = sn1;
  delay_index[sn2] = sn1;
}

void Correlator_node::add_uvw_table(Uvw_model &table, int sn) {
  if(uvw_tables.size() <= sn){
    uvw_tables.resize(sn+1);
  }
  uvw_tables[sn].add_scans(table);
}

void Correlator_node::hook_added_data_reader(size_t stream_nr) {
  // create the bit sample reader tasklet
  if (reader_thread_.bit_sample_readers().size() <= stream_nr) {
    reader_thread_.bit_sample_readers().resize(stream_nr+1, Bit_sample_reader_ptr());
    n_streams = stream_nr + 1;
  }
  reader_thread_.bit_sample_readers()[stream_nr] =
       Bit_sample_reader_ptr(new Correlator_node_data_reader_tasklet());
  reader_thread_.bit_sample_readers()[stream_nr]->connect_to(stream_nr, data_readers_ctrl.get_data_reader(stream_nr));

  // connect reader to data stream worker

  bit_statistics_ptr statistics = bit_statistics_ptr(new bit_statistics());
  bit2float_thread_.connect_to(stream_nr, statistics,
                               reader_thread_.bit_sample_readers()[stream_nr]->get_output_buffer());

  { // create the delay modules
    if (delay_modules.size() <= stream_nr) {
      delay_modules.resize(stream_nr+1,
                           boost::shared_ptr<Delay_correction>());
    }
    delay_modules[stream_nr] = Delay_correction_ptr(new Delay_correction(stream_nr));
    // Connect the delay_correction to the bits2float_converter
    delay_modules[stream_nr]->connect_to(bit2float_thread_.get_output_buffer(stream_nr));
  }


  // Connect the correlation_core to delay_correction
  correlation_core_normal->connect_to(stream_nr, statistics,
                                      delay_modules[stream_nr]->get_output_buffer());
  correlation_core_normal->connect_to(stream_nr, bit2float_thread_.get_invalid(stream_nr));
  if(pulsar_binning){
    correlation_core_pulsar->connect_to(stream_nr, statistics,
                                        delay_modules[stream_nr]->get_output_buffer());
    correlation_core_pulsar->connect_to(stream_nr, bit2float_thread_.get_invalid(stream_nr));
  }
}

void Correlator_node::hook_added_data_writer(size_t i) {
  SFXC_ASSERT(i == 0);

  correlation_core_normal->set_data_writer(data_writer_ctrl.get_data_writer(0));
  if(pulsar_binning)
    correlation_core_pulsar->set_data_writer(data_writer_ctrl.get_data_writer(0));
}

int Correlator_node::get_correlate_node_number() {
  return nr_corr_node;
}

void Correlator_node::correlate() {
  RT_STAT( dotask_state_.begin_measure() );
  bool done_work=false; 
  delay_timer_.resume();
  for (size_t i=0; i<delay_modules.size(); i++) {
    if (delay_modules[i] != Delay_correction_ptr()) {
      if (delay_modules[i]->has_work()) {
        RT_STAT( delaycorrection_state_.begin_measure() );
        delay_modules[i]->do_task();
        RT_STAT( delaycorrection_state_.end_measure(1) );
        done_work=true;
      }
    }
  }
  delay_timer_.stop();

  correlation_timer_.resume();
  if (correlation_core->has_work()) {
    RT_STAT( correlation_state_.begin_measure() );

    correlation_core->do_task();
    done_work=true;

    RT_STAT( correlation_state_.end_measure(1) );
  }

  correlation_timer_.stop();

  RT_STAT( dotask_state_.end_measure(1) );

  if (!done_work)
    usleep(1000);

}

void
Correlator_node::receive_parameters(const Correlation_parameters &parameters) {
  integration_slices_queue.push(parameters);

  /// We add the new timeslice to the readers.
  reader_thread_.add_time_slice_to_read(parameters);

  if (status == STOPPED)
    set_parameters();

}

void
Correlator_node::set_parameters() {
  SFXC_ASSERT(status == STOPPED);

  if ( !isinitialized_ ) {
    ///DEBUG_MSG("START THE THREADS !");
    isinitialized_ = true;
    start_threads();
  }

  if (integration_slices_queue.empty())
    return;

  const Correlation_parameters &parameters =
    integration_slices_queue.front();

  SFXC_ASSERT(((parameters.stop_time-parameters.start_time) /
               parameters.integration_time) == 1);
  // Get delay and UVW tables
  // NB: The total number of streams in the job is not nessecarily the same 
  // as the number of streams in the scan
  Time tmid = parameters.start_time + parameters.integration_time/2;
  std::vector<Delay_table_akima> akima_tables(delay_index.size());
  std::vector<std::vector<double> > uvw(uvw_tables.size());
  for(int i=0;i<parameters.station_streams.size();i++){
    int stream = parameters.station_streams[i].station_stream;
    int index = delay_index[stream];
    SFXC_ASSERT(index != -1);
    akima_tables[stream] = 
       delay_tables[index].create_akima_spline(parameters.start_time,
                                                parameters.integration_time);
    if(stream < uvw.size()){
      uvw[stream].resize(parameters.n_phase_centers*3);
      for(int j=0;j<parameters.n_phase_centers;j++){
        double *out = &uvw[stream][3*j];
        uvw_tables[stream].get_uvw(j, tmid, &out[0], &out[1], &out[2]);
      }
    }
  }
  int nBins=1;
  if(pulsar_binning){
    std::map<std::string, Pulsar_parameters::Pulsar>::iterator cur_pulsar_it =
                           pulsar_parameters.pulsars.find(std::string(&parameters.source[0]));
    if(cur_pulsar_it == pulsar_parameters.pulsars.end()){
      // Current source is not a pulsar
      nBins = 1;
      correlation_core = correlation_core_normal;
      correlation_core->set_parameters(parameters, akima_tables, uvw, get_correlate_node_number());
    }else{
      Pulsar_parameters::Pulsar &pulsar = cur_pulsar_it->second;
      nBins = pulsar.nbins + 1; // One extra for off-pulse data 
      correlation_core = correlation_core_pulsar;
      correlation_core_pulsar->set_parameters(parameters, pulsar, akima_tables, uvw, get_correlate_node_number());
    }
  }else{
    nBins = parameters.n_phase_centers;
    correlation_core->set_parameters(parameters, akima_tables, uvw, get_correlate_node_number());
  }

  for (size_t i=0; i<delay_modules.size(); i++) {
    if (delay_modules[i] != Delay_correction_ptr()) {
      delay_modules[i]->set_parameters(parameters, akima_tables[i]);
    }
  }
  bit2float_thread_.set_parameters(parameters, akima_tables);

  has_requested=false;
  status = CORRELATING;

  n_integration_slice_in_time_slice =
    (parameters.stop_time-parameters.start_time) / parameters.integration_time;
  // set the output stream
  int nstreams = parameters.station_streams.size();
  std::set<int> stations_set;
  for (int i = 0; i < nstreams; i++) {
    int station = parameters.station_streams[i].station_number;
    stations_set.insert(station);
  }
  int nstations = stations_set.size();
  int nBaselines = correlation_core->number_of_baselines();
  int size_of_one_baseline = sizeof(std::complex<FLOAT>) * (parameters.number_channels + 1);

  int size_uvw = nstations*sizeof(Output_uvw_coordinates);
  // when the cross_polarize flag is set then the correlator node receives 2 polarizations
  int size_stats = nstreams * sizeof(Output_header_bitstatistics);

  int slice_size;
  slice_size = nBins * ( sizeof(int32_t) + sizeof(Output_header_timeslice) + size_uvw + size_stats +
               nBaselines * ( size_of_one_baseline + sizeof(Output_header_baseline)));
  SFXC_ASSERT(nBins >= 1);
  output_node_set_timeslice(parameters.slice_nr,
                            parameters.slice_offset,
                            n_integration_slice_in_time_slice,
                            get_correlate_node_number(),slice_size, nBins);
  integration_slices_queue.pop();
}

void
Correlator_node::
output_node_set_timeslice(int slice_nr, int slice_offset, int n_slices,
                          int stream_nr, int bytes, int bins) {
  correlation_core->data_writer()->set_size_dataslice(bytes*n_slices);
  int32_t msg_output_node[] = {stream_nr, slice_nr, bytes, bins};
  for (int i=0; i<n_slices; i++) {
    MPI_Send(&msg_output_node, 4, MPI_INT32,
             RANK_OUTPUT_NODE,
             MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY,
             MPI_COMM_WORLD);
    msg_output_node[1] += slice_offset;
  }
}
