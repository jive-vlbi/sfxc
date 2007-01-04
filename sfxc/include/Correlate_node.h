#ifndef CORRELATE_NODE_H
#define CORRELATE_NODE_H

#include <Node.h>
#include <Input_controller.h>
#include <Correlate_controller.h>
#include <Output_controller.h>

#include <Semaphore_buffer.h>


class Correlate_node : public Node
{
public:
  Correlate_node(int rank, int buff_size=10);
  ~Correlate_node();
  
private:
  // Buffer for the output, input is directly handled by the Correlate_controller
  Semaphore_buffer<Output_controller::value_type> buffer;

  Correlate_controller correlate_controller;
  Output_controller    output_controller;
};

#endif // CORRELATE_NODE_H
