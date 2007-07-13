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
          if (bytes_to_write > ch_buffer_size) {
            size_t result = ch_buffer_size;
            do {
              result -= writer->put_bytes(result, ch_buffer);
            } while (result != 0);
          } else {
            size_t result = bytes_to_write;
            do {
              result -= writer->put_bytes(result, ch_buffer);
            } while (result != 0);

            assert(writer->end_of_dataslice());
            std::list<int>::iterator tmp = it;
            it++;
            remove_from_active_list(*tmp);
            it --;
          }
        }

        // refill the buffer and set the current time stamp:
        fill_channel_buffer();

        update_active_list();
        if (active_list.empty()) status = STOPPED;
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
  while ((!start_queue.empty()) &&
         (start_queue.begin()->first <= get_time_stamp())) {
    assert(start_queue.begin()->first == get_time_stamp());
    assert(data_writers_ctrl.get_data_writer(start_queue.begin()->second)->get_size_dataslice()!=0);
    add_to_active_list(start_queue.begin()->second);
    start_queue.erase(start_queue.begin());
  }
}  

void Input_node::add_to_active_list(int stream) {
  // Debugging code:
  for (std::list<int>::iterator it = active_list.begin();
       it != active_list.end(); it++) {
    assert(*it != stream);
  }
  
  active_list.push_back(stream);
  
  assert(data_writers_ctrl.buffer(stream) == NULL);
}

void Input_node::remove_from_active_list(int stream) {
  if (get_rank() == 4) {
    get_log_writer()(0)
      << "Finished transfer of data to node " 
      << stream << std::endl;
  }
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

  // fill the buffer and set the current time stamp:
  time_stamp = channel_extractor->get_current_time();
  size_t size = channel_extractor->get_bytes(ch_buffer_size, ch_buffer);
  if (size != ch_buffer_size) {
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
  assert(get_time_stamp() <= new_time);
  channel_extractor->goto_time(new_time);
  time_stamp = channel_extractor->get_current_time();
  assert(get_time_stamp() == new_time);
}
