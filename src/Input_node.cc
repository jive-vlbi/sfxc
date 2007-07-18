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

#include <MPI_Transfer.h>

#define BUFFER_SIZE 10

Input_node::Input_node(int rank, int station_number, Log_writer *log_writer) 
  : Node(rank, log_writer), 
    input_node_ctrl(*this),
    data_reader_ctrl(*this),
    data_writers_ctrl(*this),
    buffer_size(BUFFER_SIZE),
    nr_input_reader(station_number),
    status(STOPPED),
    stop_time(-1)
{
  initialise();
}
Input_node::Input_node(int rank, int station_number) 
  : Node(rank), 
    input_node_ctrl(*this),
    data_reader_ctrl(*this),
    data_writers_ctrl(*this),
    buffer_size(BUFFER_SIZE),
    nr_input_reader(station_number),
    status(STOPPED),
    stop_time(-1)
{
  initialise();
}
void Input_node::initialise() 
{
  get_log_writer() << "Input_node()" << std::endl;

  // NGHK: These use the global parameters: change
  MPI_Status status;
  MPI_Probe(RANK_MANAGER_NODE, MPI_TAG_CONTROL_PARAM, MPI_COMM_WORLD, &status);
  get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
  MPI_Transfer mpi_transfer;
  mpi_transfer.receive_general_parameters(status,RunPrms,GenPrms,StaPrms);
  
  add_controller(&input_node_ctrl);
  add_controller(&data_reader_ctrl);
  add_controller(&data_writers_ctrl);

  int32_t msg;
  MPI_Send(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);

}


Input_node::~Input_node() {
  assert(active_list.empty());
}

int64_t Input_node::get_time_stamp() {
  assert (channel_extractor != NULL);
  assert(time_stamp <= channel_extractor->get_current_time());
  return time_stamp;
}

int32_t Input_node::get_delta_time() {
  // NGHK: CHANGE THIS IMMEDIATELY make it dependent on the fan out and the number of tracks
  assert(ch_buffer_size*10000%(2*frameMk4)==0); 
  return ch_buffer_size*10000/(2*frameMk4); 
}



void Input_node::start() {
  while (status != END_NODE) {
    switch (status) {
      case STOPPED: {
        // blocking:
        if (check_and_process_message()==TERMINATE_NODE) {
          status = END_NODE;
        }
        if (!active_list.empty()) status = SEND_OUTPUT;
        break;
      }
      case SEND_OUTPUT: {
        MESSAGE_RESULT msg_result = check_and_process_waiting_message();
        if (msg_result != NO_MESSAGE) {
          while (msg_result != NO_MESSAGE) {
            if (msg_result == TERMINATE_NODE) {
              status = END_NODE;
              break;
            }
            msg_result = check_and_process_waiting_message();
          }
          if ((status != END_NODE) && active_list.empty()) {
            status = STOPPED;
          }
          if (status != SEND_OUTPUT) break;
        } 
        
        assert(!active_list.empty());
        assert(status == SEND_OUTPUT);
        for (std::list<int>::iterator it = active_list.begin();
             it != active_list.end(); it++) {
          boost::shared_ptr<Data_writer> 
            writer = data_writers_ctrl.get_data_writer(*it);
          assert(writer != boost::shared_ptr<Data_writer>());
          size_t bytes_to_write = writer->get_size_dataslice();
          if (bytes_to_write >= ch_buffer_size) {
            size_t result = 0;
            while (result != ch_buffer_size) {
              result += writer->put_bytes(ch_buffer_size-result, 
                                          ch_buffer+result);
            };
          } else {
            // Delete stream
            if (bytes_to_write != 0) {
              size_t result = 0;
              while (result != bytes_to_write) {
                result += writer->put_bytes(bytes_to_write-result, 
                                            ch_buffer+result);
              };
            }

            assert(writer->end_of_dataslice());
            std::list<int>::iterator tmp = it;
            it++;
            remove_from_active_list(*tmp);
            it --;
          }
        }

        // Add new streams
        if (active_list.empty()) {
          // Duplicate update_active_list() is needed,
          // because of fill_channel_buffer()
          update_active_list();
          if (active_list.empty()) status = STOPPED;
        } else {
          // Duplicate update_active_list() is needed,
          // because of fill_channel_buffer()
          update_active_list();
          // refill the buffer and set the current time stamp:
          fill_channel_buffer();
        }


        break;
      }
      case END_NODE: { // For completeness sake, is caught by the while loop
        break;
      }
    }
    if (active_list.empty()) {
      if ((stop_time >= 0) && (stop_time <= get_time_stamp())) {
        status = END_NODE;
      }
    }
  }

  while (!data_writers_ctrl.ready()) usleep(100000); // .1 second:


  while (!active_list.empty()) {
    remove_from_active_list(*active_list.begin());
  } 
  
  int32_t rank = get_rank();
  MPI_Send(&rank, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD);
}

void 
Input_node::
set_priority(int stream, int slicenr, uint64_t start, uint64_t stop) {
  assert((start <= stop) || (stop == 0));
  start_queue.insert(std::pair<int64_t,int>(start, stream));
  // if stop == 0, then output to the stream until the end
  assert(data_writers_ctrl.get_data_writer(stream)->get_size_dataslice()<=0);
  int bytes = -1;
  if (stop > 0) {
    // 2*bwfl is the Nyquist sample rate
    // two bits sampling, hence 4 samples per byte
    int bits_per_sample = 2;
    int NYQUIST = 2;
    bytes = (int)((BufTime*4 + (stop-start))* 
                  (GenPrms.get_bwfl()*NYQUIST*bits_per_sample/8)/1000000);
  }
  data_writers_ctrl.get_data_writer(stream)->set_size_dataslice(bytes);

  update_active_list();
}

void Input_node::update_active_list() {
  // Check start_queue:
  bool do_fill_buffer = false;

  while ((!start_queue.empty()) &&
         (start_queue.begin()->first < (get_time_stamp()+get_delta_time()))) {

    if (active_list.empty()) {
      do_fill_buffer = true;
    }

    add_to_active_list(start_queue.begin()->first, 
                       start_queue.begin()->second);

    start_queue.erase(start_queue.begin());
  }
  
  if (do_fill_buffer) {
    fill_channel_buffer();
  }
}  

void Input_node::add_to_active_list(int64_t start_time, int stream) {
  assert(start_time >= get_time_stamp());
  assert(start_time < get_time_stamp() + get_delta_time());


  // Debugging code:
  for (std::list<int>::iterator it = active_list.begin();
       it != active_list.end(); it++) {
    assert(*it != stream);
  }
  
  // make sure the start falls on an integer byte
  assert((ch_buffer_size*(start_time-get_time_stamp())) % get_delta_time()
         == 0);

  int start_byte = 
    (ch_buffer_size*(start_time-get_time_stamp())) / get_delta_time();
  assert(start_byte >= 0);
  assert(start_byte < ch_buffer_size);

  boost::shared_ptr<Data_writer> 
    writer = data_writers_ctrl.get_data_writer(stream);

  int bytes_to_write = ch_buffer_size-start_byte;
  assert(bytes_to_write > 0);
  assert(bytes_to_write < writer->get_size_dataslice());

  int result = 0;
  do {
    result += writer->put_bytes(bytes_to_write-result, 
                                &ch_buffer[start_byte+result]);
  } while (result != bytes_to_write);

  // add the stream to the list of active streams
  active_list.push_back(stream);
}

void Input_node::remove_from_active_list(int stream) {
  bool found = false;
  for (std::list<int>::iterator it = active_list.begin();
       it != active_list.end(); it++) {
    if (*it == stream) {
      std::list<int>::iterator to_delete = it;
      it ++;
      active_list.erase(to_delete);
      it --;
      found = true;
    }
  }
  assert(found);
}

void Input_node::hook_added_data_reader(size_t stream_nr) {
  assert(channel_extractor == boost::shared_ptr<Channel_extractor>());
  assert(data_reader_ctrl.get_data_reader() != NULL);
  channel_extractor = boost::shared_ptr<Channel_extractor>(
      new Channel_extractor_mark4(data_reader_ctrl.get_data_reader(), 
                                  StaPrms[nr_input_reader], 
                                  GenPrms.get_rndhdr()));

  fill_channel_buffer();
}

void Input_node::fill_channel_buffer() {
  // No input buffer:
  assert(data_reader_ctrl.buffer() == NULL);

  // fill the buffer and set the current time stamp
#if 0 // Check delta time
  int64_t new_time = channel_extractor->get_current_time();
  if (new_time-time_stamp != get_delta_time()) {
    DEBUG_MSG("Delta time not correct: " 
              << new_time-time_stamp << " != " << get_delta_time())
  }
  time_stamp = new_time;
#else 
  time_stamp = channel_extractor->get_current_time();
#endif


  size_t size = channel_extractor->get_bytes(ch_buffer_size, ch_buffer);
  if (size != ch_buffer_size) {
    assert(false);
    status = END_NODE;
    // do write the remaining bytes
  }
}

void Input_node::hook_added_data_writer(size_t writer) {
}

void Input_node::set_stop_time(int64_t stop_time_) {
  stop_time = stop_time_;
}

void Input_node::goto_time(int64_t new_time) {
  // NGHK: CHECK FOR DURATION OF THE CHANNEL BUFFER
  assert(get_time_stamp() <= new_time);
  channel_extractor->goto_time(new_time);
  assert(channel_extractor->get_current_time() == new_time);
  fill_channel_buffer();
  assert(get_time_stamp() == new_time);
}
