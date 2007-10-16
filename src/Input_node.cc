/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Input_node.h>

#include <types.h>
#include <Semaphore_buffer.h>

#include <Data_reader_buffer.h>
#include <Channel_extractor_mark4.h>

#include <iostream>
#include <assert.h>
#include <time.h>
#include <math.h>

#include <MPI_Transfer.h>

Input_node::Input_node(int rank, int station_number, Log_writer *log_writer) 
  : Node(rank, log_writer), 
    input_node_ctrl(*this),
    data_reader_ctrl(*this),
    data_writers_ctrl(*this),
    status(WAITING),
    stop_time(-1)
{
  initialise();
}
Input_node::Input_node(int rank, int station_number) 
  : Node(rank), 
    input_node_ctrl(*this),
    data_reader_ctrl(*this),
    data_writers_ctrl(*this),
    status(WAITING),
    stop_time(-1)
{
  initialise();
}
void Input_node::initialise()  {
  get_log_writer() << "Input_node()" << std::endl;
  add_controller(&input_node_ctrl);
  add_controller(&data_reader_ctrl);
  add_controller(&data_writers_ctrl);

  int32_t msg;
  MPI_Send(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);

}

void 
Input_node::
set_track_parameters(const Track_parameters &track_param) {
  assert(channel_extractor != 
         boost::shared_ptr<Channel_extractor_mark4>());
  channel_extractor->set_track_parameters(track_param);

  // #buffered blocks = Max_delay * byte_rate per channel / bytes per block
  int buffered_elements = 
    static_cast<int>(ceil((MAX_DELAY * 1. *
                           ((channel_extractor->track_bit_rate()/8)) / 
                           channel_extractor->number_of_bytes_per_block()) /
                          1000));
  int total_elements = buffered_elements + 5;

  // remove old time slicers:
  time_slicers.resize(0, Slicer(total_elements, buffered_elements));

  // create time slicers
  time_slicers.resize(track_param.channels.size(), 
                      Slicer(total_elements, buffered_elements));
}

Input_node::~Input_node() {
}

int64_t Input_node::get_time_stamp() {
  assert (channel_extractor != NULL);
  return channel_extractor->get_current_time();
}

void Input_node::start() {
  static int prev_status = -1;
  while (status != END_NODE) {
    if (status != prev_status) {
      DEBUG_MSG("Input_node::status: " << status);
      prev_status = status;
    }
    switch (status) {
    case WAITING:
      { // Wait until we can start sending new data
        if (channel_extractor != 
            boost::shared_ptr<Channel_extractor_mark4>()) {
          if (channel_extractor->get_current_time() < stop_time) {
            status = INITIALISING;
          } else {
            if (check_and_process_message() == TERMINATE_NODE) {
              status = END_NODE;
              break;
            }
          }
        } else {
          // Wait for data_source to become ready
          if (check_and_process_message() == TERMINATE_NODE) {
            status = END_NODE;
            break;
          }
        }
        break;
      }
    case INITIALISING: 
      { // Wait untill a data writer has been connected to all channels
        bool ready = true;
        for (int i=0; i<channel_extractor->n_channels(); i++) {
          ready &= !time_slicers[i].finished();
        }
        if (ready) {
          status = WRITING;
        } else {
          // Block until the next message arrives
          if (check_and_process_message() == TERMINATE_NODE) {
            status = END_NODE;
          }
        }
        break;
      }
    case WRITING: 
      {
        if (process_all_waiting_messages() == TERMINATE_NODE) {
          status = END_NODE;
          break;
        }
        bool read = false;
        // Check whether we reached the end:
        if (channel_extractor->get_current_time() < stop_time) {
          // Check whether all buffers have storage available
          bool storage_available = true;
          for (int i=0; i<channel_extractor->n_channels(); i++) {
            storage_available &= !time_slicers[i].full();
          }
          if (storage_available) {
            // Process one block
            std::vector < char * > buffers;
            buffers.resize(channel_extractor->n_channels());
          
            for (int i=0; i<channel_extractor->n_channels(); i++) {
              buffers[i] = time_slicers[i].produce().buffer();
            }
            int bytes = channel_extractor->get_bytes(buffers);
            channel_extractor->goto_next_block();
            // Release the buffers
            for (int i=0; i<channel_extractor->n_channels(); i++) {
              time_slicers[i].produced(bytes);
            }
            read = true;
          }
        }
        // Write the output
        bool wrote = true;
        for (int i=0; i<channel_extractor->n_channels(); i++) {
          wrote &= time_slicers[i].do_task();
        }
        if (!wrote) {
          if (!read) status = WAITING;
          status = INITIALISING;
        }
        break;
      }
    case END_NODE:
      {
        assert(false);
      }
    }
  }

  while (!data_writers_ctrl.ready()) usleep(100000); // .1 second:

  int32_t rank = get_rank();
  MPI_Send(&rank, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD);
}

void Input_node::hook_added_data_reader(size_t stream_nr) {
  channel_extractor = 
    boost::shared_ptr<Channel_extractor_mark4>
    (new Channel_extractor_mark4(data_reader_ctrl.get_data_reader(stream_nr), 
                                 /*insert_random_headers*/ true
                                 /*, Debug_level debug_level*/));
}

void Input_node::hook_added_data_writer(size_t writer) {
}

void Input_node::set_stop_time(int64_t stop_time_) {
  stop_time = stop_time_;
}

void Input_node::goto_time(int64_t new_time) {
  // NGHK: CHECK FOR DURATION OF THE CHANNEL BUFFER
  assert(get_time_stamp() <= new_time);
  
  channel_extractor->goto_time(new_time/*-MAX_DELAY*/);
  start_time = new_time/*-MAX_DELAY*/;
  // NGHK: TODO: Somehow empty the time slicers
  assert(channel_extractor->get_current_time() == new_time/*-MAX_DELAY*/);
}

void 
Input_node::
add_time_slice(int channel, int stream, 
               int starttime_slice, int stoptime_slice) {
  //starttime_slice -= MAX_DELAY; // Needed for the delay correction
  stoptime_slice += MAX_DELAY; // Needed for the delay correction
  int start_byte = 
    ((starttime_slice-start_time) * 
     (channel_extractor->bit_rate(channel)/8000));
  int bytes_in_slice =
    ((stoptime_slice-starttime_slice) * 
     (channel_extractor->bit_rate(channel)/8000));

  assert(data_writers_ctrl.get_data_writer(stream) != NULL);
  assert(data_writers_ctrl.get_data_writer(stream)->get_size_dataslice() <= 0);

  data_writers_ctrl.get_data_writer(stream)->
    set_size_dataslice(bytes_in_slice);
  time_slicers[channel].add(data_writers_ctrl.get_data_writer(stream), 
                            start_byte);
}

int Input_node::get_status() {
  return status;
}
