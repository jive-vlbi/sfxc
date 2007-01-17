/*  
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#ifndef DATA_NODE_H
#define DATA_NODE_H

#include <Node.h>
#include <Input_controller.h>
#include <Output_controller.h>
#include <Semaphore_buffer.h>
#include <Ring_buffer.h>

#include "Log_writer_cout.h"

class Data_node : public Node {
public:
  Data_node(int rank, int buffer_size = 1024);
  ~Data_node();

private:
  //Ring_buffer<Input_controller::value_type> buffer;
  Semaphore_buffer<Input_controller::value_type> buffer;

  Log_writer_cout log_writer;

  Input_controller input;
  Output_controller output;
};

#endif // DATA_NODE_H
