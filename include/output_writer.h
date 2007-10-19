#ifndef OUTPUT_WRITER_H
#define OUTPUT_WRITER_H

#include <boost/shared_ptr.hpp>

#include "tasklet/component.h"
#include "Queue_buffer.h"
#include "Data_writer_file.h"

class Output_writer : public Component {
public:
  typedef Input_reader::Output_buffer_element  Input_buffer_element;
  typedef Input_reader::Output_buffer          Input_buffer;
  typedef Input_reader::Output_buffer_ptr      Input_buffer_ptr;

  Output_writer(TaskletPool &work_queue, 
                char filename[]);
  ~Output_writer();

  void do_task();
  void do_execute();

  void connect_to(Input_buffer_ptr input_buffer);

private:
  Input_buffer_ptr input_buffer;  
  Data_writer_file data_writer;
};

#endif // OUTPUT_WRITER_H
