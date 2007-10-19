#ifndef INPUT_READER_H
#define INPUT_READER_H

#include <boost/shared_ptr.hpp>

#include "tasklet/component.h"
#include "Queue_buffer.h"
#include "Data_reader_file.h"

class Input_reader : public Component {
public:
  struct Output_buffer_element {
    Buffer_element_large<char, 1024> element;
    size_t size;
    size_t command;
  };
  typedef Queue_buffer< Output_buffer_element > Output_buffer;
  typedef boost::shared_ptr<Output_buffer>      Output_buffer_ptr;

  Input_reader(TaskletPool &work_queue, 
               char filename[]);
  ~Input_reader();

  void do_task();
  void do_execute();

  Output_buffer_ptr get_output_buffer();

private:
  Output_buffer_ptr output_buffer;  
  Data_reader_file  data_reader;
};

#endif // INPUT_READER_H
