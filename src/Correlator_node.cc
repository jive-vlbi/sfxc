#include "Correlator_node.h"

#include "ProcessData.h"
#include "InData.h"

Correlator_node::Correlator_node(int rank, int buff_size)
 : Node(rank),
   output_buffer(buff_size),
   correlator_controller(*this),
   output_controller(output_buffer, log_writer),
   correlate_state(INITIALISE_TIME_SLICE), 
   status(STOPPED)
{
  log_writer.MPI(0, "Correlate_node(rank)");
  //add_controller(&output_controller);
  set_log_writer(log_writer);

  add_controller(&correlator_controller);
  int i=0;
  MPI_Send(&i, 1, MPI_INT, 0, MPI_TAG_CORRELATE_ENDED, MPI_COMM_WORLD);
}

Correlator_node::~Correlator_node()
{
  log_writer.message(1, "~Correlate_node()");
}

void Correlator_node::add_data_reader(Data_reader *reader) {
  data_readers.push_back(reader);
}
void Correlator_node::set_data_writer(Data_writer *data_writer_) {
  if (data_writer != NULL) delete data_writer;
  data_writer = data_writer_;
}


void Correlator_node::start()
{
  int err;
  while (status != END_CORRELATING) {
    switch (status) {
      case STOPPED: {
        // blocking:
        if (check_and_process_messages()==-1) {
          status = END_CORRELATING;
        }
        break;
      }
      case CORRELATING: {
        while (check_and_process_waiting_messages() && (status==CORRELATING)) {}
        
        if (status==CORRELATING) {
          switch(correlate_state) {
            case FIND_INITIAL_OFFSETS: {
              //Find Offsets
              correlate_state = INITIALISE_TIME_SLICE;
              if (FindOffsets(data_readers, 1, 0) !=0) {
                log_writer.message(0,"ERROR: FindOffsets, program aborted.\n");
                correlate_state = END_TIME_SLICE;
              }
              break;
            }
            case INITIALISE_TIME_SLICE: {
              log_writer(0) << "Initialising correlation\n";
              err = CorrelateBufs_initialise(data_readers);
              log_writer(0) << "/Initialising correlation\n";
              correlate_state = CORRELATE_SEGMENT;              
              if (err < 0) {
                log_writer.error("Initialising correlation");
                status = END_CORRELATING;
              }
              break;
            }
            case CORRELATE_SEGMENT: {
              log_writer(0) << "Process segment\n";
              err = CorrelateBufs_process_segment();
              log_writer(0) << "/Process segment\n";
              if (err != 0) {
                if (err > 0) {
                  correlate_state = END_TIME_SLICE;
                } else {
                  log_writer.error("During correlation");
                  status = END_CORRELATING;
                }
              }
              break;
            }
            case END_TIME_SLICE: {
              log_writer(0) << "Finalise\n";
              err = CorrelateBufs_finalise();
              log_writer(0) << "/Finalise\n";
              status = STOPPED;
              if (err < 0) {
                log_writer.error("in end time slice");
                status = END_CORRELATING;
              }
              int i=0;
              // Notify output node: 
//              MPI_Send(&i, 1, MPI_INT, 1,
//                       MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED, MPI_COMM_WORLD);
              // Notify manager node:
              MPI_Send(&i, 1, MPI_INT, 0,
                       MPI_TAG_CORRELATE_ENDED, MPI_COMM_WORLD);
              break;
            }
          }
        }
      }
    }
  }
}

