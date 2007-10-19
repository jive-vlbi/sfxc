#include "output_writer.h"

Output_writer::Output_writer(TaskletPool &work_queue, 
                             char filename[]) 
: Component(work_queue), data_writer(filename) {
  
}
Output_writer::~Output_writer() {
}

void Output_writer::connect_to(Input_buffer_ptr new_input_buffer) {
  input_buffer = new_input_buffer;
}

void Output_writer::do_task() {
  std::cout << "do_task not yet implemented" << std::endl;
}

void Output_writer::do_execute() {
  int size; 
  int i=0;
  do {
    Input_buffer_element &elem = input_buffer->consume(size);
    std::cout << "OUTPUT " << i << " " << size << std::endl;
    input_buffer->consumed();
    i++;
  } while (size > 0);
}
