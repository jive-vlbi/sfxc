#include "correlator_node_data_reader_tasklet.h"

Correlator_node_data_reader_tasklet::
Correlator_node_data_reader_tasklet()
    :output_memory_pool(4000),//old version 65000
    output_buffer(Output_buffer_ptr(new Output_buffer())),
    n_bytes_to_read(0) {
}

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
  int32_t n_bytes; 
  int n_invalid_blocks,n_delays;

  Output_memory_pool_element output_elem = output_memory_pool.allocate();

  // Start_time of the data
  breader_->get_bytes(sizeof(output_elem.data().start_time), (char*)&output_elem.data().start_time);
  // Number of bytes of data
  breader_->get_bytes(sizeof(n_bytes), (char*)&n_bytes);

  // invalid block data
  breader_->get_bytes(sizeof(output_elem.data().invalid_samples_begin),
                     (char*)&output_elem.data().invalid_samples_begin);
  breader_->get_bytes(sizeof(output_elem.data().nr_invalid_samples),
                      (char*)&output_elem.data().nr_invalid_samples);
  // the delay in samples
  int bytes_read=breader_->get_bytes(sizeof(output_elem.data().delay), (char*)&output_elem.data().delay);

  if(bytes_read==0) // eof reached
    return;

  if (output_elem.data().data.size() != n_bytes)
    output_elem.data().data.resize(n_bytes);

  // Read in the data
  breader_->get_bytes(n_bytes, (char*)&output_elem.data().data[0]);

  n_bytes_to_read -= n_bytes;
  output_buffer->push(output_elem);
}

uint64_t Correlator_node_data_reader_tasklet::data_to_read() {
  return n_bytes_to_read;
}

bool Correlator_node_data_reader_tasklet::has_work() {
  if (n_bytes_to_read <= 0)
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
  uint64_t n_bytes_per_fft = (number_channels_*bits_per_sample_)/8;
  n_bytes_to_read += n_ffts_to_read_*n_bytes_per_fft;
}
