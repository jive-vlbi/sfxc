#include "Correlator_node.h"

#include "ProcessData.h"
#include "Data_writer_buffer.h"
#include "InData.h"

Correlator_node::Correlator_node(int rank, int buff_size)
 : Node(rank),
   output_buffer(buff_size),
   correlator_node_ctrl(*this),
   data_readers_ctrl(*this),
   data_writer_ctrl(*this),
   correlate_state(INITIALISE_TIME_SLICE),
   status(STOPPED), initial_slice(true)
{
  get_log_writer().MPI(0, "Correlate_node(rank)");
  //add_controller(&output_controller);
  
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
  while (!data_writer_ctrl.buffer()->empty()) {
    usleep(100000);
  }
  get_log_writer().message(1, "~Correlate_node()");
}

void Correlator_node::start()
{
  int err;
  while (status != END_CORRELATING) {
    switch (status) {
      case STOPPED: {
        // blocking:
        if (check_and_process_message()==TERMINATE_NODE) {
          status = END_CORRELATING;
        }
        break;
      }
      case CORRELATING: {
        while ((check_and_process_waiting_message() != NO_MESSAGE) &&
               (status==CORRELATING)) {
        }
        
        if (status==CORRELATING) {
          switch(correlate_state) {
            case FIND_INITIAL_OFFSETS: {
              if (initial_slice) {
                initial_slice = false;
                for (unsigned int i=0; 
                     i<data_readers_ctrl.number_of_data_readers(); i++) {
                  if (data_readers_ctrl.initialised(i)) {
                    data_readers_ctrl.set_buffer(i, 
                      new Semaphore_buffer<input_value_type>(1000));
                  }
                }
              }
              //Find Offsets
              correlate_state = INITIALISE_TIME_SLICE;
              err = FindOffsets(data_readers_ctrl.get_vector_data_readers(), 1, 0);
              if (err !=0) {
                get_log_writer().message(0,"ERROR: FindOffsets, program aborted.\n");
                correlate_state = END_TIME_SLICE;
              }
              break;
            }
            case INITIALISE_TIME_SLICE: {
              err = CorrelateBufs_initialise_correlator(data_readers_ctrl.get_vector_data_readers());
              assert(err == 0);
              err = CorrelateBufs_initialise_time_slice();
              correlate_state = CORRELATE_SEGMENT;              
              if (err < 0) {
                get_log_writer().error("Initialising correlation");
                status = END_CORRELATING;
              }
              break;
            }
            case CORRELATE_SEGMENT: {
              err = CorrelateBufs_process_segment();
              if (err != 0) {
                if (err > 0) {
                  correlate_state = END_TIME_SLICE;
                } else {
                  get_log_writer().error("During correlation");
                  status = END_CORRELATING;
                }
              }
              break;
            }
            case END_TIME_SLICE: {
              err = CorrelateBufs_finalise();
              status = STOPPED;
              if (err < 0) {
                get_log_writer().error("in end time slice");
                status = END_CORRELATING;
              }
              INT32 i=0;
              // Notify output node: 
              MPI_Send(&i, 1, MPI_INT32, RANK_OUTPUT_NODE,
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


void Correlator_node::hook_added_data_reader(int reader) {
}
void Correlator_node::hook_added_data_writer(int i) {
  assert(i == 0);
  
  data_writer_ctrl.set_buffer(new Semaphore_buffer<output_value_type>(100));
  
  Data_writer *writer = new Data_writer_buffer(data_writer_ctrl.buffer());
  ::set_data_writer(*writer);
}
