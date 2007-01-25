/* Author(s): Nico Kruithof, 2007
 * 
 * $URL:$
 * $Id: $
 */

#ifndef OUTPUT_NODE_H
#define OUTPUT_NODE_H

#include <Node.h>
#include <Input_controller.h>
#include <Output_controller.h>
#include <Semaphore_buffer.h>
#include <Ring_buffer.h>

#include "Log_writer_cout.h"

class Output_node : public Node {
public:
  Output_node(int rank, int buffer_size = 1024);
  ~Output_node();

private:
  //Ring_buffer<Input_controller::value_type> buffer;
  Semaphore_buffer<Input_controller::value_type> buffer;

  Input_controller input;
  Output_controller output;
};

#endif // OUTPUT_NODE_H
