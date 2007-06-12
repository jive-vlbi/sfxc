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
#include "InData.h"
#include <Channel_extractor_mark4.h>

Correlator_node::Correlator_node(int rank, int nr_corr_node, int buff_size)
 : Node(rank),
   output_buffer(buff_size),
   correlator_node_ctrl(*this),
   data_readers_ctrl(*this),
   data_writer_ctrl(*this),
   integration_slice(get_log_writer()),
   correlate_state(INITIALISE_TIME_SLICE),
   status(STOPPED),
   buffer_size(buff_size), nr_corr_node(nr_corr_node),
   startIS(-1)
{
  get_log_writer() << "Correlator_node()" << std::endl;;
  
  // set the log writer for ProcessData:
  ::set_log_writer(get_log_writer());

  add_controller(&correlator_node_ctrl);
  add_controller(&data_readers_ctrl);
  add_controller(&data_writer_ctrl);
  
  INT32 msg;
  MPI_Send(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);
  
  int i=0;
  MPI_Send(&i, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED, MPI_COMM_WORLD);
}

Correlator_node::~Correlator_node()
{
}

void Correlator_node::start()
{
  while (true) {
    switch (status) {
      case END_CORRELATING: {
        return;
      }
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
        
        if (status==CORRELATING) {
          switch(correlate_state) {
            case INITIALISE_TIME_SLICE: {
              get_log_writer()(2) << " correlate_state = INITIALISE_TIME_SLICE" << std::endl;
              // Initialise the correlator for a new time slice:
              
              startIS=GenPrms.get_usStart();

              //initialise readers to proper position
              Init_reader_struct init_readers[GenPrms.get_nstations()];
              for (int sn=0; sn<GenPrms.get_nstations(); sn++) {
                init_readers[sn].corr_node = this;
                init_readers[sn].startIS = startIS;
                init_readers[sn].sn = sn;

                pthread_create(&init_readers[sn].thread, NULL, 
                               start_init_reader, static_cast<void*>(&init_readers[sn]));
              }
              for (int sn=0; sn<GenPrms.get_nstations(); sn++) {
                pthread_join(init_readers[sn].thread, NULL);
              }
              
              correlate_state = CORRELATE_INTEGRATION_SLICE;
              break;
            }
            case CORRELATE_INTEGRATION_SLICE: {
              get_log_writer()(2) << " correlate_state = CORRELATE_INTEGRATION_SLICE" << std::endl;
              // Do one integration step:
              //while still time slices and data are available
              if (startIS >= GenPrms.get_usStart() + GenPrms.get_usDur()) {
                correlate_state = END_TIME_SLICE;
                break;
              }
                             
              //process the next time slice:
              get_integration_slice().correlate();
              //set start of next time slice to: start of time slice + time to average
              startIS += GenPrms.get_usTime2Avg(); //in usec
              break;
            }
            case END_TIME_SLICE: {
              get_log_writer()(2) << " correlate_state = END_TIME_SLICE" << std::endl;

              // Finish processing a time slice:
              status = STOPPED;
              UINT64 i[] = {get_correlate_node_number(), 
                            get_data_writer().data_counter()};
              get_data_writer().reset_data_counter();
              // Notify output node: 
              MPI_Send(&i, 2, MPI_UINT64, RANK_OUTPUT_NODE,
                       MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED, 
                       MPI_COMM_WORLD);
              // Notify manager node:
              INT32 msg = 0;
              MPI_Send(&msg, 1, MPI_INT32, RANK_MANAGER_NODE,
                       MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED, 
                       MPI_COMM_WORLD);
                       
  
                       
              break;
            }
          }
        }
      }
    }
  }
}

void Correlator_node::start_correlating(INT64 us_start, INT64 duration) {
  assert(status != CORRELATING); 
  
  GenPrms.set_usStart(us_start);
  GenPrms.set_duration(duration);

  get_integration_slice().set_start_time(us_start);


  status=CORRELATING; 
  correlate_state = INITIALISE_TIME_SLICE; 
}

void Correlator_node::add_delay_table(int sn, DelayTable &table) {
  get_integration_slice().set_delay_table(sn, table);
}


void Correlator_node::set_parameters(RunP &runPrms, GenP &genPrms, StaP *staPrms) {
  get_integration_slice().set_parameters(genPrms, staPrms, 
					 runPrms.get_ref_station(0),
					 runPrms.get_ref_station(1));
}


void Correlator_node::hook_added_data_reader(size_t stream_nr) {
  Bits_to_float_converter *sample_reader = new Bits_to_float_converter();
  sample_reader->set_bits_per_sample(StaPrms->get_bps());
  sample_reader->set_data_reader(data_readers_ctrl.get_data_reader(stream_nr));
  
  get_integration_slice().set_sample_reader(stream_nr,sample_reader);
}

void Correlator_node::hook_added_data_writer(size_t i) {
  assert(i == 0);

  get_integration_slice().set_data_writer(data_writer_ctrl.get_data_writer(i));
}

int Correlator_node::get_correlate_node_number() {
  return nr_corr_node;
}

void Correlator_node::set_slice_number(int sliceNr_) {
  sliceNr = sliceNr_;
}

void *Correlator_node::start_init_reader(void * self_) {
  Init_reader_struct *ir_struct = static_cast<Init_reader_struct *>(self_);
  Correlator_node *node = ir_struct->corr_node;
//  node->channel_extractors[ir_struct->sn]->goto_time(ir_struct->startIS);
  node->get_integration_slice().init_reader(ir_struct->sn,ir_struct->startIS);
  return NULL;
}
