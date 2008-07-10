#include "correlator_node_data_reader_tasklet.h"

Correlator_node_data_reader_tasklet::
Correlator_node_data_reader_tasklet()
    : output_memory_pool(65000),
    output_buffer(Output_buffer_ptr(new Output_buffer())),
    n_ffts_to_read(0) {}

Correlator_node_data_reader_tasklet::
~Correlator_node_data_reader_tasklet() {}

/// Set the input
void
Correlator_node_data_reader_tasklet::
connect_to(Data_reader_ptr reader_) {
  reader = reader_;
  breader_ = Data_reader_blocking_ptr( new Data_reader_blocking( reader_.get() ) );
}

Correlator_node_data_reader_tasklet::Output_buffer_ptr
Correlator_node_data_reader_tasklet::
get_output_buffer() {
  return output_buffer;
}

void Correlator_node_data_reader_tasklet::do_task() {
  SFXC_ASSERT(has_work());

  Output_memory_pool_element output_elem = output_memory_pool.allocate();
  // allocate the data array
  if (output_elem.data().bytes_count() != (size_t)n_bytes_per_fft) {
    output_elem.data().resize_bytes_buffer(n_bytes_per_fft);
  }

  // Read in one fft, with the extra data for invalid samples
  breader_->get_bytes(sizeof(output_elem.data().invalid_samples_begin),
                      (char*)&output_elem.data().invalid_samples_begin);
  breader_->get_bytes(sizeof(output_elem.data().nr_invalid_samples),
                      (char*)&output_elem.data().nr_invalid_samples);
  breader_->get_bytes(output_elem.data().raw_size(),
                      (char*)output_elem.data().raw_buffer());

  n_ffts_to_read --;


  output_buffer->push(output_elem);
}

int Correlator_node_data_reader_tasklet::data_to_read() {
  return n_ffts_to_read;
}

bool Correlator_node_data_reader_tasklet::has_work() {
  if (n_ffts_to_read <= 0)
    return false;

  if (reader == Data_reader_ptr())
    return false;

  if (output_memory_pool.empty())
    return false;

  if (!reader->can_read())
    return false;

  return true;
}

int Correlator_node_data_reader_tasklet::get_fd() {
  return reader->get_fd();
}

void
Correlator_node_data_reader_tasklet::
set_parameters(const int n_ffts_to_read_,
               const int bits_per_sample_,
               const int number_channels_) {
  n_ffts_to_read += n_ffts_to_read_;
  n_bytes_per_fft = (number_channels_*bits_per_sample_)/8 + 1;
}
