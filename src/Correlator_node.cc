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
//  assert(data_writer_ctrl.buffer() != NULL);
//  while (!data_writer_ctrl.buffer()->empty()) {
//    usleep(100000);
//  }
}

void Correlator_node::start()
{
  while (true) {
    switch (status) {
      case END_CORRELATING: {
        return;
      }
      case STOPPED: {
//        get_log_writer() << " status = STOPPED" << std::endl;
        // blocking:
        if (check_and_process_message()==TERMINATE_NODE) {
          status = END_CORRELATING;
        }
        break;
      }
      case CORRELATING: {
        get_log_writer() << " status = CORRELATING" << std::endl;
        if (process_all_waiting_messages() == TERMINATE_NODE) {
          status = END_CORRELATING;
        }
        
        if (status==CORRELATING) {
          switch(correlate_state) {
            case INITIALISE_TIME_SLICE: {
              get_log_writer() << " correlate_state = INITIALISE_TIME_SLICE" << std::endl;
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
              get_log_writer() << " correlate_state = CORRELATE_INTEGRATION_SLICE" << std::endl;
              // Do one integration step:
              //while still time slices and data are available
              if (startIS >= GenPrms.get_usStart() + GenPrms.get_usDur()
                     /* && data_available TODO RHJO implement*/ ) {
                correlate_state = END_TIME_SLICE;
                break;
              }
                             
              //process the next time slice:
              integration_slice.correlate();
              //set start of next time slice to: start of time slice + time to average
              startIS += GenPrms.get_usTime2Avg(); //in usec
              break;
            }
            case END_TIME_SLICE: {
              get_log_writer() << " correlate_state = END_TIME_SLICE" << std::endl;
//              int size = (256+1);
//              fftw_complex buff[size];
//              for (int i=0; i<size; i++) {
//                buff[i][0] = sliceNr;
//                buff[i][1] = sliceNr;
//              }
//              integration_slice.cc.get_data_writer().put_bytes(sizeof(fftw_complex)*size, (char *)buff);

              // Finish processing a time slice:
              status = STOPPED;
              UINT64 i[] = {get_correlate_node_number(), 
                            get_data_writer().data_counter()};
              get_data_writer().reset_data_counter();
              // Notify output node: 
              MPI_Send(&i, 2, MPI_UINT64, RANK_OUTPUT_NODE,
                       MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED, MPI_COMM_WORLD);
              // Notify manager node:
              INT32 msg = 0;
              MPI_Send(&msg, 1, MPI_INT32, RANK_MANAGER_NODE,
                       MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED, MPI_COMM_WORLD);
                       
  
                       
              break;
            }
          }
        }
      }
    }
  }
}

void Correlator_node::start_correlating(INT64 start, INT64 duration) {
  assert(status != CORRELATING); 
  
  GenPrms.set_usStart(start);
  GenPrms.set_duration(duration);
  
  status=CORRELATING; 
  correlate_state = INITIALISE_TIME_SLICE; 
}

void Correlator_node::add_delay_table(int sn, DelayTable &table) {
  integration_slice.set_delay_table(sn, table);
}


void Correlator_node::set_parameters(RunP &runPrms, GenP &genPrms, StaP *staPrms) {
  integration_slice.set_parameters(genPrms, staPrms, RunPrms.get_ref_station());
}


void Correlator_node::hook_added_data_reader(int i) {
//  Buffer<input_value_type> *buffer = new Semaphore_buffer<input_value_type>(buffer_size);
//  data_readers_ctrl.set_buffer(i, buffer);
//  integration_slice.set_data_reader(i,new Data_reader_buffer(buffer));
  integration_slice.set_data_reader(i,data_readers_ctrl.get_data_reader(i));
}

void Correlator_node::hook_added_data_writer(int i) {
  assert(i == 0);
//  data_writer_ctrl.set_buffer(new Semaphore_buffer<output_value_type>(buffer_size));
//  integration_slice.set_data_writer(new Data_writer_buffer(data_writer_ctrl.buffer()));
  integration_slice.set_data_writer(data_writer_ctrl.get_data_writer(i));
}

int Correlator_node::get_correlate_node_number() {
  return nr_corr_node;
}

void Correlator_node::set_slice_number(int sliceNr_) {
  sliceNr = sliceNr_;
}

void *Correlator_node::start_init_reader(void * self_) {
  Init_reader_struct *ir_struct = static_cast<Init_reader_struct *>(self_);
  ir_struct->corr_node->integration_slice.init_reader(ir_struct->sn,ir_struct->startIS);
  return NULL;
}
