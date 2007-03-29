#include "Correlator_node.h"

#include "Data_writer_buffer.h"
#include "InData.h"

Correlator_node::Correlator_node(int rank, int buff_size)
 : Node(rank),
   output_buffer(buff_size),
   correlator_node_ctrl(*this),
   data_readers_ctrl(*this),
   data_writer_ctrl(*this),
   integration_slice(GenPrms, StaPrms, get_log_writer()),
   correlate_state(INITIALISE_TIME_SLICE),
   status(STOPPED),
   buffer_size(buff_size)
{
  get_log_writer().MPI(0, "Correlate_node(rank)");
  
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
           RANK_MANAGER_NODE, MPI_TAG_CORRELATE_ENDED, MPI_COMM_WORLD);
}

Correlator_node::~Correlator_node()
{
  assert(data_writer_ctrl.buffer() != NULL);
  while (!data_writer_ctrl.buffer()->empty()) {
    usleep(100000);
  }
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
        if (process_all_waiting_messages() == TERMINATE_NODE) {
          status = END_CORRELATING;
        }
        
        if (status==CORRELATING) {
          switch(correlate_state) {
            case INITIALISE_TIME_SLICE: {
              // Initialise the correlator for a new time slice:
              startIS=GenPrms.get_usStart();
              correlate_state = CORRELATE_SEGMENT;
              break;
            }
            case CORRELATE_SEGMENT: {
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
              get_log_writer() << "Time slice finished" << std::endl;
              // Finish processing a time slice:
              status = STOPPED;
              UINT64 i[] = {get_rank(), get_data_writer().data_counter()};
              get_data_writer().reset_data_counter();
              // Notify output node: 
              MPI_Send(&i, 2, MPI_UINT64, RANK_OUTPUT_NODE,
                       MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED, MPI_COMM_WORLD);
              // Notify manager node:
              MPI_Send(&i, 1, MPI_INT32, RANK_MANAGER_NODE,
                       MPI_TAG_CORRELATE_ENDED, MPI_COMM_WORLD);
              break;
            }
          }
        }
      }
    }
  }
}

void Correlator_node::start_correlating() {
  assert(status != CORRELATING); 
  status=CORRELATING; 
  correlate_state = INITIALISE_TIME_SLICE; 
}

void Correlator_node::add_delay_table(int sn, DelayTable &table) {
  integration_slice.set_delay_table(sn, table);
}


void Correlator_node::hook_added_data_reader(int i) {
  Buffer<input_value_type> *buffer = new Semaphore_buffer<input_value_type>(buffer_size);
  data_readers_ctrl.set_buffer(i, buffer);
}

void Correlator_node::hook_added_data_writer(int i) {
  assert(i == 0);
  
  data_writer_ctrl.set_buffer(new Semaphore_buffer<output_value_type>(buffer_size));
  
  integration_slice.set_data_writer(new Data_writer_buffer(data_writer_ctrl.buffer()));
}
