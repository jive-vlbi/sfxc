#ifndef CORRELATOR_NODE_H
#define CORRELATOR_NODE_H

#include <Node.h>
#include <Input_controller.h>
#include <Correlator_controller.h>
#include <Output_controller.h>

#include <Semaphore_buffer.h>

#include "Log_writer_cout.h"
#include "Log_writer_void.h"

class Correlator_node : public Node
{
public:
  Correlator_node(int rank, int buff_size=10);
  ~Correlator_node();
  
private:
  // Buffer for the output, input is directly handled by the Correlator_controller
  Semaphore_buffer<Output_controller::value_type> buffer;

  Correlator_controller correlator_controller;
  Output_controller     output_controller;
};

#endif // CORRELATOR_NODE_H
