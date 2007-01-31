/* Author(s): Nico Kruithof, 2007
 * 
 * $URL:$
 * $Id: $
 */

#ifndef OUTPUT_NODE_H
#define OUTPUT_NODE_H

#include <Node.h>
#include <Output_controller.h>
#include <Semaphore_buffer.h>
#include <Ring_buffer.h>

#include <Log_controller.h>
#include <Log_writer_cout.h>

class Output_node : public Node {
public:
  typedef Buffer_element<char,131072>    value_type;

  Output_node(int rank, int buffer_size = 1024);
  ~Output_node();

private:
//  //Ring_buffer<Input_controller::value_type> buffer;
  Semaphore_buffer<value_type> buffer;

  Log_writer_cout log_writer_cout;
  
  Output_controller output;
};

#endif // OUTPUT_NODE_H
