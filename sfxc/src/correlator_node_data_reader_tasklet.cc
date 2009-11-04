#include "correlator_node_data_reader_tasklet.h"

Correlator_node_data_reader_tasklet::
Correlator_node_data_reader_tasklet()
    : input_buffer(8100000), bytes_left(0), //output_memory_pool(4000),//old version 65000
    new_stream_available(false),state(IDLE) {
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

Correlator_node_data_reader_tasklet::Input_buffer_ptr
Correlator_node_data_reader_tasklet::
get_output_buffer() {
  return &input_buffer;
}

void Correlator_node_data_reader_tasklet::do_task() {
  SFXC_ASSERT(has_work());
  uint8_t header;
  // For convinience
  std::vector<unsigned char> &data = input_buffer.data;
  int dsize = data.size();
  uint64_t &read = input_buffer.read;
  uint64_t &write = input_buffer.write;
  switch(state){
  case IDLE:
    if(!new_stream_available){
      return;
    }
    new_stream_available = false;
    state = PROCESSING_STREAM;
  case PROCESSING_STREAM:
    breader_->get_bytes(sizeof(header), (char *)&header);
    data[write%dsize]=header;
    write++;
    switch(header){
    case HEADER_DATA:{
      int16_t nbytes;
      breader_->get_bytes(sizeof(nbytes), (char *)&nbytes);
      char *nbytes_buffer = (char *)&nbytes;
      data[write%dsize]=nbytes_buffer[0];
      write++;
      data[write%dsize]=nbytes_buffer[1];
      write++;
      bytes_left = nbytes;
      state = RECEIVE_DATA;
      break;
    }case HEADER_DELAY:{
      int8_t new_delay;
      breader_->get_bytes(sizeof(new_delay), (char *)&new_delay);
      SFXC_ASSERT((new_delay >= 0)&&(new_delay <samples_per_byte));
      data[write%dsize]=new_delay;
      write++;
      break;
    }case HEADER_INVALID:{
      int16_t n_invalid;
      breader_->get_bytes(sizeof(n_invalid), (char *)&n_invalid);
      char *invalid_buffer=(char *)&n_invalid;
      data[write%dsize]=invalid_buffer[0];
      write++;
      data[write%dsize]=invalid_buffer[1];
      write++;
      break;
    }case HEADER_ENDSTREAM:
      state = IDLE;
      break;
    default:
      SFXC_ASSERT_MSG(false, "Read invalid header from data stream");
    }
    break;
  case RECEIVE_DATA:
    if(bytes_left>0){
      int bytes_left_in_buffer = input_buffer.bytes_free();
      int to_read = std::min(bytes_left, bytes_left_in_buffer-1);
      int data_read=0;
      while(data_read < to_read){
        int data_to_read = std::min(to_read - data_read, (int)(data.size()-write%dsize));
        breader_->get_bytes(data_to_read, (char *)&data[write%dsize]);
        data_read += data_to_read;
        write = write+data_to_read;
      }
      bytes_left  -= to_read;
    }
    if(bytes_left==0){
      state = PROCESSING_STREAM;
    }
    SFXC_ASSERT(bytes_left >=0);
    break;
  }
}

bool Correlator_node_data_reader_tasklet::has_work() {
  if (reader == Data_reader_ptr()){
    return false;
  }

  if (!reader->can_read()){
    return false;
  }

  if(input_buffer.bytes_free() < INPUT_BUFFER_MINIMUM_FREE){
    return false;
  }

  if (state == IDLE){
    return new_stream_available;
  }

  return true;
}

int Correlator_node_data_reader_tasklet::get_fd() {
  return reader->get_fd();
}

bool Correlator_node_data_reader_tasklet::active() {
  if(state!=IDLE)
    return true;
  else
    return new_stream_available;
}
void
Correlator_node_data_reader_tasklet::
set_parameters(const int n_ffts_to_read_,
               const int bits_per_sample_,
               const int number_channels_) {
  bits_per_sample=bits_per_sample_;
  samples_per_byte = 8/bits_per_sample;
  new_stream_available = true;
}
