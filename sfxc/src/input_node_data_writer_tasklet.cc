#include "input_node_data_writer_tasklet.h"

Input_node_data_writer_tasklet::
Input_node_data_writer_tasklet() {

}

Input_node_data_writer_tasklet::~Input_node_data_writer_tasklet() {
  if (input_buffer_ != Input_buffer_ptr()) {
    if (!input_buffer_->empty()) {
      DEBUG_MSG("There is still data to be written. "
                << input_buffer_->size());
    }
  }
  while  (!data_writers_.empty()) {
    if ((data_writers_.front().writer->get_size_dataslice() <= 0) &&
        (data_writers_.front().slice_size == 0)) {
      data_writers_.pop();
    } else {
      break;
    }
  }
  if (!data_writers_.empty()) {
    DEBUG_MSG("Data_writers are still waiting to produce output.");
  }
}

void
Input_node_data_writer_tasklet::
connect_to(Input_buffer_ptr new_input_buffer) {
  input_buffer_ = new_input_buffer;
}

bool
Input_node_data_writer_tasklet::
has_work() {
  if (input_buffer_->empty())
    return false;

  if (data_writers_.empty())
    return false;

  if (!data_writers_.front().writer->can_write()) {
    //DEBUG_MSG("Data writer can't write");
    return false;
  }
  return true;
}

void
Input_node_data_writer_tasklet::
do_task() {
  assert(has_work());

  if (data_writers_.front().slice_size > 0) {
    // Initialise the size of the data slice
    assert(data_writers_.front().writer->get_size_dataslice() <= 0);
    int nr_bytes = data_writers_.front().slice_size;
    assert(nr_bytes != 0);
    data_writers_.front().writer->set_size_dataslice(nr_bytes);
    data_writers_.front().slice_size = 0;
  }
  assert(data_writers_.front().writer->get_size_dataslice() >= 0);
  if (data_writers_.front().writer->get_size_dataslice() == 0) {
    data_writers_.pop();
    return;
  }

  Input_buffer_element &input_element = input_buffer_->front();

  if (input_element.release_data) {
    input_element.channel_data.release();
    input_buffer_->pop();
    return;
  }
  if ((int)input_element.delay >= 0) {
    int nbytes = 0;
    do {
      nbytes = data_writers_.front().writer->put_bytes(1, &input_element.delay);
    } while (nbytes != 1);
  }

  int bytes_to_write = input_element.nr_samples;
  int bytes_written = 0;
  char *data =
    (char*)&input_element.channel_data.data().data[input_element.first_sample];

  while (bytes_written < bytes_to_write) {
    int nbytes =
      data_writers_.front().writer->put_bytes(bytes_to_write - bytes_written, data);
    assert(nbytes >= 0);
    bytes_written += nbytes;
    data          += nbytes;
  }

  input_buffer_->pop();
}

void
Input_node_data_writer_tasklet::
add_data_writer(Data_writer_ptr data_writer, int nr_bytes) {
  Writer_struct writer;
  writer.writer = data_writer;
  writer.slice_size = nr_bytes;
  data_writers_.push(writer);
}

void
Input_node_data_writer_tasklet::
set_parameters(const Input_node_parameters &input_param) {}

