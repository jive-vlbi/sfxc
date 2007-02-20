/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#include <Input_node.h>

#include <types.h>
#include <Semaphore_buffer.h>

#include <iostream>
#include <assert.h>
#include <time.h>

Input_node::Input_node(int rank, Log_writer *log_writer, int size) 
  : Node(rank, log_writer), 
    input_node_ctrl(*this),
    data_reader_ctrl(get_log_writer()),
    data_writers_ctrl(get_log_writer()),
    time_stamp(1),
    buffer_size(size),
    status(STOPPED)
{
  initialise();
}
Input_node::Input_node(int rank, int size) 
  : Node(rank), 
    input_node_ctrl(*this),
    data_reader_ctrl(get_log_writer()),
    data_writers_ctrl(get_log_writer()),
    time_stamp(1),
    buffer_size(size),
    status(STOPPED)
{
  initialise();
}
void Input_node::initialise() 
{
  data_reader_ctrl.set_buffer(new Buffer(buffer_size));
  get_log_writer().MPI(0, "Input_node()");
  add_controller(&input_node_ctrl);
  add_controller(&data_reader_ctrl);
  add_controller(&data_writers_ctrl);

  INT32 msg;
  MPI_Send(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);
}


Input_node::~Input_node() {
  while (!active_list.empty()) {
    stop_stream(active_list.begin());
  } 
}

void Input_node::set_status() {
  if (status == END_NODE) return;
  
  if (data_reader_ctrl.eof()) {
    status = END_NODE;
  } else if (!active_list.empty()) {
    status = SEND_OUTPUT;
  } else {
    status = STOPPED;
  }
}

void Input_node::start() {
  while (status != END_NODE) {
    switch (status) {
      case STOPPED: {
        // blocking:
        if (check_and_process_message()==TERMINATE_NODE) {
          status = END_NODE;
        }
        break;
      }
      case SEND_OUTPUT: {
        while ((check_and_process_waiting_message() != NO_MESSAGE) &&
               (status != END_NODE)) {
        }
        
        if (data_reader_ctrl.eof() && data_reader_ctrl.buffer()->empty()) {
          status = END_NODE;
          break;
        }
        if (status == SEND_OUTPUT) {
          if (data_reader_ctrl.buffer()->empty()) {
            usleep(100000); // .1 second:
          } else {
            int size;
            assert(data_reader_ctrl.buffer() != NULL);
            value_type &cons_elem = data_reader_ctrl.buffer()->consume(size);

            update_active_list();

            assert(!active_list.empty());
            for (std::list<int>::iterator it = active_list.begin();
                 it != active_list.end(); it++) {
              assert(data_writers_ctrl.buffer(*it) != NULL);
              value_type &prod_elem =
                data_writers_ctrl.buffer(*it)->produce();
              memcpy(prod_elem.buffer(), cons_elem.buffer(), size);
              data_writers_ctrl.buffer(*it)->produced(size);
            }
            data_reader_ctrl.buffer()->consumed();
          }
        }
        break;
      }
      case END_NODE: { // For completeness sake, is caught by the while loop
        break;
      }
    }
  }
  
  while (!data_writers_ctrl.ready()) {
    usleep(100000); // .1 second:
  }
  
  INT32 rank = get_rank();
  MPI_Send(&rank, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD);
}


void Input_node::set_priority(int stream, UINT64 start, UINT64 stop) {
  start_queue.insert(std::pair<INT64,int>(start, stream));
  // if stop == 0, then output to the stream until the end
  if (stop > 0) stop_queue.insert(std::pair<UINT64,int>(stop, stream));
  
  update_active_list();
}

void Input_node::set_time_stamp(value_type &t) {
  std::cout << "TIME STAMP NOT SET" << std::endl;
}  

void Input_node::update_active_list() {
  // Check start_queue:
  while (!start_queue.empty() &&
         start_queue.begin()->first < get_time_stamp()) {
    int stream = start_queue.begin()->second;
    add_to_active_list(stream);
    start_queue.erase(start_queue.begin());
  }

  // Check stop_queue:
  while (!stop_queue.empty() &&
         stop_queue.begin()->first < get_time_stamp()) {
    bool found = false;
    int stream = stop_queue.begin()->second;
    for (std::list<int>::iterator it = active_list.begin();
         it != active_list.end(); it++) {
      if (*it == stream) {
        it--;
        stop_stream(it);
        found = true;
      }
    }
    assert(found);
    stop_queue.erase(stop_queue.begin());
  }
  set_status();
}  

void Input_node::add_to_active_list(int stream) {
  // Debugging code:
  for (std::list<int>::iterator it = active_list.begin();
       it != active_list.end(); it++) {
    assert(*it != stream);
  }
  
  active_list.push_back(stream);
  assert(data_writers_ctrl.buffer(stream) == NULL);
  data_writers_ctrl.set_buffer(stream, new Buffer(buffer_size));
}

void Input_node::remove_from_active_list(int stream) {
  bool found = false;
  for (std::list<int>::iterator it = active_list.begin();
       it != active_list.end(); it++) {
    if (*it == stream) {
      assert(!found);
      if (it == active_list.begin()) {
        active_list.erase(it);
        it = active_list.begin();
      } else {
        std::list<int>::iterator it_del = it;
        it--;
        active_list.erase(it_del);
      }
      found = true;
    }
  }
  assert(found);
}

void Input_node::stop_stream(const std::list<int>::iterator &stream_it) {
  INT32 rank = get_rank();
  MPI_Send(&rank, 1, MPI_INT32, 
           *stream_it, MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED,
           MPI_COMM_WORLD);
  
  std::list<int>::iterator it_del = stream_it;
  active_list.erase(it_del);
}

