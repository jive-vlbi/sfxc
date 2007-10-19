#include "input_reader.h"

Input_reader::Input_reader(TaskletPool &work_queue, 
                           char filename[]) 
: Component(work_queue), data_reader(filename) {
  output_buffer = Output_buffer_ptr(new Output_buffer());
}
Input_reader::~Input_reader() {
}

Input_reader::Output_buffer_ptr 
Input_reader::get_output_buffer() {
  return output_buffer;
}

void Input_reader::do_task() {
  std::cout << "read not yet implemented" << std::endl;
}

void Input_reader::do_execute() {
  int i=0;
  while (!data_reader.eof()) {
    Output_buffer_element &elem = output_buffer->produce();
    size_t size = data_reader.get_bytes(elem.size(), elem.buffer());
    assert(size > 0);
    std::cout << "INPUT " << i << " " << size << std::endl;
    output_buffer->produced(size);
    //usleep(10000);
    i++;
  }

  // last message
  output_buffer->produce();
  output_buffer->produced(0);
}
