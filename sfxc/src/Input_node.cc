/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#include <Input_node.h>

#include <types.h>

#include <iostream>
#include <assert.h>

Input_node::Input_node(int rank, int size) 
  : Node(rank), 
    buffer(size), 
    input_controller(*this),
    status(STOPPED)
{
  get_log_writer().MPI(0, "Input_node(rank,size)");
  add_controller(&input_controller);
}

Input_node::~Input_node() {
  log_writer(1) << "~Input_node()";
}

void Input_node::set_status() {
  if (status == END_NODE) return;
  
  if ((data_reader != NULL) && (data_writers.size() > 0)) status = SEND_OUTPUT;
  else status = STOPPED;
   
}


void Input_node::set_data_writer(int pos, Data_writer *writer) {
  if (data_writers.size() <= (unsigned int)pos) data_writers.resize(pos+1, NULL);
  if (data_writers[pos] != NULL) delete(data_writers[pos]);
  data_writers[pos] = writer;
}
Data_writer *Input_node::get_data_writer(int pos) {
  return data_writers[pos];
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
               (status != END_NODE)) {}
        break;
      }
      case END_NODE: {
        break;
      }
    }
  }
}

