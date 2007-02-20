#include "Correlator_node.h"

#include "ProcessData.h"
#include "InData.h"

Correlator_node::Correlator_node(int rank, int buff_size)
 : Node(rank),
   output_buffer(buff_size),
   data_writer(NULL), 
   correlator_controller(*this),
   //output_controller(*this),
   correlate_state(INITIALISE_TIME_SLICE),
   status(STOPPED)
{
  get_log_writer().MPI(0, "Correlate_node(rank)");
  //add_controller(&output_controller);
  
  // set the log writer for ProcessData:
  ::set_log_writer(get_log_writer());

  add_controller(&correlator_controller);
  int i=0;
  MPI_Send(&i, 1, MPI_INT32, 0, MPI_TAG_CORRELATE_ENDED, MPI_COMM_WORLD);
}

Correlator_node::~Correlator_node()
{
  get_log_writer().message(1, "~Correlate_node()");
}

void Correlator_node::add_data_reader(Data_reader *reader) {
  data_readers.push_back(reader);
}

void Correlator_node::set_data_reader(int node, Data_reader *reader) {
  if (data_readers.size() <= (UINT32)node) data_readers.resize(node+1, NULL);
  if (data_readers[node] != NULL) delete data_readers[node];
  data_readers[node] = reader;
}
void Correlator_node::set_data_writer(Data_writer *data_writer_) {
  if (data_writer != NULL) delete data_writer;
  data_writer = data_writer_;
  assert(data_writer != NULL);
  
  // Set the data writer in ProcessData:
  ::set_data_writer(*data_writer);
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
              //Find Offsets
              correlate_state = INITIALISE_TIME_SLICE;
              err = FindOffsets(data_readers, 1, 0);
              if (err !=0) {
                get_log_writer().message(0,"ERROR: FindOffsets, program aborted.\n");
                correlate_state = END_TIME_SLICE;
              }
              break;
            }
            case INITIALISE_TIME_SLICE: {
              err = CorrelateBufs_initialise_correlator(data_readers);
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
              int i=0;
              // Notify output node: 
              MPI_Send(&i, 1, MPI_INT32, 1,
                       MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED, MPI_COMM_WORLD);
              // Notify manager node:
              MPI_Send(&i, 1, MPI_INT32, 0,
                       MPI_TAG_CORRELATE_ENDED, MPI_COMM_WORLD);
              break;
            }
          }
        }
      }
    }
  }
}

