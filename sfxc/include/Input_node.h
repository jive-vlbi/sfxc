/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#ifndef INPUT_NODE_H
#define INPUT_NODE_H

#include <Node.h>
#include <Input_controller.h>

#include <Data_reader.h>
#include <Data_writer.h>

#include <Semaphore_buffer.h>
#include <Ring_buffer.h>

#include "Log_writer_cout.h"

class Input_node : public Node {
public:
  Input_node(int rank, int buffer_size = 1024);
  ~Input_node();
  
  void start();

  enum STATUS {
    STOPPED=0,
    SEND_OUTPUT,
    END_NODE
  };
  
  void set_status();
  
  void set_data_writer(int pos, Data_writer *writer);
  Data_writer *get_data_writer(int pos);
  
private:
  Semaphore_buffer<Input_controller::value_type> buffer;

  Input_controller            input_controller;
  Data_reader                *data_reader;
  std::vector<Data_writer *>  data_writers;
  
  STATUS status;
};

#endif // INPUT_NODE_H
