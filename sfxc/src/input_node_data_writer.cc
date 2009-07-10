#include <math.h>
#include <limits.h>
#include "input_node_data_writer.h"
Input_node_data_writer::
Input_node_data_writer(){
	last_duration_ = 0;
	total_data_written_ = 0;
  delay_index=0;
  _current_time=INVALID_TIME;
  interval=0;
}

Input_node_data_writer::~Input_node_data_writer() {
  if (input_buffer_ != Input_buffer_ptr()) {
      if (!input_buffer_->empty()) {
          DEBUG_MSG("There is still data to be written. "
                    << input_buffer_->size());
        }
    }
  while  (!data_writers_.empty()) {
      if ((data_writers_.front().active) &&
          (data_writers_.front().slice_size <= 0)) {
          data_writers_.pop();
        } else {
          break;
        }
    }
  if (!data_writers_.empty()) {
      DEBUG_MSG("Data_writers are still waiting to produce output.");
    }

  double wait_duration = (timer_waiting_.measured_time()+timer_other_.measured_time());
  double total_duration = wait_duration+timer_writing_.measured_time();
	double ratio1 = ((100.0*timer_waiting_.measured_time())/total_duration);
	double ratio2 = ((100.0*timer_other_.measured_time())/total_duration);
	double ratio3 = ((100.0*timer_writing_.measured_time())/total_duration);

	last_duration_ = total_duration;
	DEBUG_MSG( "data_writer byte sent:" << toMB(total_data_written_) << "MB" );

}

void
Input_node_data_writer::
connect_to(Input_buffer_ptr new_input_buffer) {
  input_buffer_ = new_input_buffer;
}

bool
Input_node_data_writer::
has_work() {
  // Check for new time interval
  if ( _current_time >= current_interval_.stop_time_ ) {
    if (( intervals_.empty() ) || ( delays_.empty() )){
     return false;
    }
    interval++;
    fetch_next_time_interval();
  }

 // Not sufficient input data
  if(input_buffer_->empty())
    return false;
  else if ((input_buffer_->size()<2)&&
           (current_interval_.stop_time_ - _current_time  >
            input_buffer_->front().channel_data.data().data.size()*time_per_byte) )
    return false;


  // No data writers to send the data to
  if (data_writers_.empty())
    return false;

  // The data writer in the front of the queue is still being used
  // to send data from another channel
  //TODO check if it is possible that the same data_writer is used multiple times
  if ((!data_writers_.front().active) && (data_writers_.front().writer->is_active()))
    return false;

  // Check whether we can send data to the active writer
  if (!data_writers_.front().writer->can_write())
    return false;

  return true;
}

uint64_t
Input_node_data_writer::
do_task() {

  SFXC_ASSERT(has_work());

  // Acquire the input data
  Input_buffer_element &input_element = input_buffer_->front();
  struct Writer_struct& data_writer = data_writers_.front();

  // Check whether we have to start a new timeslice
  if(!data_writer.active){  
    // Initialise the size of the data slice
    // from the front(): writer.set_size_dataslice(slice_size), slice_size=0
    SFXC_ASSERT(data_writer.slice_size>0);
    data_writer.writer->set_size_dataslice(-1);
    data_writer.writer->activate();
    data_writer.active = true;
    // Set the current time to the beginning of the integration slice
    _current_time = ((_current_time+time_fft)/integration_time)*integration_time;

    DEBUG_MSG("FETCHING FOR A NEW WRITER......");
  }

  // Check whether we have written all data to the data_writer
  if (data_writer.slice_size == 0) {
      data_writer.writer->deactivate();
      data_writers_.pop();
      DEBUG_MSG("POPPING FOR A NEW WRITER......");
      return 0;
  }
  //go to the correct positition in the delay table
  std::vector<Delay> &cur_delay = delay_list.data();
  if((delay_index<cur_delay.size()-1)&&(cur_delay[delay_index+1].time<_current_time+time_fft))
    delay_index++;

  // Obtain the amount of data to write
  int64_t dtime = (int64_t)(_current_time-input_element.start_time);
  int byte_offset = dtime*sample_rate*bits_per_sample/8/1000000 +
                    cur_delay[delay_index].bytes;

  Data_writer_sptr writer = data_writer.writer;
  block_size=input_element.channel_data.data().data.size();
  int data_to_write;

  if(byte_offset <= -INPUT_NODE_PACKET_SIZE){
    //The requested output lies completely before the input data, send invalid data
    data_to_write = std::min(INPUT_NODE_PACKET_SIZE, data_writer.slice_size+1);
    int invalid_start=0, nr_inv_samples=data_to_write*8/bits_per_sample, delay=0;
    write_header(writer,data_to_write, invalid_start, nr_inv_samples, delay);
    write_random_data(writer,data_to_write);
  }else if (byte_offset >= block_size){
    // This can happen when we go to a next integration slice
    // And the integer delay changes at the same time
    // Release the current block
    input_buffer_->pop();
    return 0;
  }else{
    int invalid_bytes, valid_bytes;
    int invalid_start, nr_inv_samples, delay;
    int block_inv_samples=input_element.nr_invalid_samples;

    int next_delay_pos=get_next_delay_pos(cur_delay, _current_time);
    data_to_write = bytes_to_write(byte_offset, next_delay_pos);
    SFXC_ASSERT(data_to_write -1 <= next_delay_pos);
    // Check if there was enough data to send 
    if(data_to_write==1)
      return 0;

    SFXC_ASSERT(data_to_write-1 <= data_writers_.front().slice_size);
    SFXC_ASSERT((data_to_write-1)%fftsize==0);
    SFXC_ASSERT(data_to_write<=INPUT_NODE_PACKET_SIZE);

    if(byte_offset<0){
      invalid_bytes=-byte_offset;
      valid_bytes=data_to_write-invalid_bytes;
      nr_inv_samples=invalid_bytes*8/bits_per_sample + 
                      std::min(valid_bytes*8/bits_per_sample, block_inv_samples);
      invalid_start=0;
      byte_offset=0;
    }else{
      invalid_bytes=0;
      valid_bytes=data_to_write;

      int second_block_start = block_size - byte_offset;
      int sample_offset = byte_offset*8/bits_per_sample;
      if( data_to_write < second_block_start){
        invalid_start = 0;
        nr_inv_samples = std::max(block_inv_samples - sample_offset, 0);
      }else{
        invalid_start = second_block_start*8/bits_per_sample;
        nr_inv_samples = std::min(data_to_write*8/bits_per_sample - invalid_start, block_inv_samples);
      }
    }
    delay=cur_delay[delay_index].remaining_samples;
    write_header(writer, data_to_write, invalid_start, nr_inv_samples, delay);
    if(invalid_bytes>0) write_random_data(writer, invalid_bytes);
    write_data(writer, valid_bytes, byte_offset);
  }

  total_data_written_ += data_to_write;
  data_writer.slice_size-=(data_to_write-1); // -1 because we allways send 1 extra byte of data

  _current_time += (uint64_t)(data_to_write-1)*8*1000000/(sample_rate*bits_per_sample);
  return data_to_write-1;
}

void
Input_node_data_writer::
add_timeslice(Data_writer_sptr data_writer, int nr_bytes) {
  Writer_struct writer;
  writer.writer = data_writer;
  writer.slice_size = nr_bytes;

  data_writers_.push(writer);
  DEBUG_MSG(": This data writer has a waiting queue of " << data_writers_.size() 
            << " writers, slice_size = " << writer.slice_size );
}

void
Input_node_data_writer::
set_parameters(const Input_node_parameters &input_param) {
  sample_rate = input_param.sample_rate();
  bits_per_sample = input_param.bits_per_sample();
  fftsize = input_param.number_channels*bits_per_sample/8; // size of one fft window in bytes
  time_fft = (((int64_t)input_param.number_channels)*1000000)/sample_rate;
  time_per_byte=(((int64_t)(8/bits_per_sample))*1000000)/sample_rate;
  integration_time = input_param.integr_time*1000;
}

// Empty the input queue, called from the destructor of Input_node
void Input_node_data_writer::empty_input_queue() {
  while (!input_buffer_->empty()) {
      input_buffer_->pop();
    }
  while (!delays_.empty()) {
      delays_.pop();
    }
}

Input_node_data_writer_sptr Input_node_data_writer::new_sptr()
{
    return Input_node_data_writer_sptr(new Input_node_data_writer());
}

void
Input_node_data_writer::add_delay(Delay_memory_pool_element delay)
{
  delays_.push(delay);
}

void
Input_node_data_writer::add_time_interval(uint64_t start, uint64_t stop) {
  SFXC_ASSERT( start < stop );
  intervals_.push( Time_interval(start, stop) );
}

void
Input_node_data_writer::fetch_next_time_interval() {
  // We retreive the current interval
  current_interval_ = intervals_.front_and_pop();
  SFXC_ASSERT( !current_interval_.empty() );
  delay_index = 0;
  delay_list = delays_.front_and_pop();

  _current_time = current_interval_.start_time_;
}

void
Input_node_data_writer::write_header(Data_writer_sptr writer, int32_t ndata, int inv_start, int nr_inv, int delay)
{
  SFXC_ASSERT((delay>=0)&&(delay<8/bits_per_sample));
  // The start time of the block
  int nbytes = writer->put_bytes(sizeof(_current_time), (char*)&_current_time);
  SFXC_ASSERT(nbytes == sizeof(_current_time));
  // No of bytes in block
  nbytes = writer->put_bytes(sizeof(ndata), (char*)&ndata);
  SFXC_ASSERT(nbytes == sizeof(ndata));
  //The invalid block parameters
  nbytes = writer->put_bytes(sizeof(inv_start), (char*)&inv_start);
  SFXC_ASSERT(nbytes == sizeof(inv_start));
  nbytes = writer->put_bytes(sizeof(nr_inv), (char*)&nr_inv);
  SFXC_ASSERT(nbytes == sizeof(nr_inv));
  // The integer delay in number of samples
  nbytes = writer->put_bytes(sizeof(delay), (char*)&delay);
  SFXC_ASSERT(nbytes == sizeof(delay));
}

int 
Input_node_data_writer::get_next_delay_pos(std::vector<Delay> &cur_delay, uint64_t start_time){
  int delay_pos;
  int delay_size=cur_delay.size();
  if(delay_index < delay_size-1){
    uint64_t dtime = cur_delay[delay_index+1].time-start_time;
    delay_pos = dtime * sample_rate*bits_per_sample/8/1000000;
  }
  else
    delay_pos=INT_MAX/2; 

  return std::max(delay_pos,0);
}

void
Input_node_data_writer::write_data(Data_writer_sptr writer, int ndata, int byte_offset)
{
  SFXC_ASSERT(byte_offset>=0);
  int buffer_size=input_buffer_->front().channel_data.data().data.size();
  int bytes_written=0;

  int start=byte_offset;

  while(bytes_written < ndata){
    Input_buffer_element &input_element = input_buffer_->front();
    char *data =(char*)&input_element.channel_data.data().data[start];
    int towrite=std::min((int)ndata-bytes_written, buffer_size-start);
    int written = 0;

    while(written < towrite){
      written += writer->put_bytes(towrite, data);
      bytes_written+=written;
      data+=written;
    }
    start=0;
    if((bytes_written+byte_offset)%buffer_size==0){
      input_buffer_->pop();
    }
  }
}

void
Input_node_data_writer::write_random_data(Data_writer_sptr writer, int ndata)
{
  int bytes_written=0;

  init_random_block(random_data_, ndata);

  //send the actual data
  while(bytes_written < ndata)
   bytes_written += writer->put_bytes(ndata-bytes_written, &random_data_[bytes_written]);
}


int
Input_node_data_writer::bytes_to_write(int byte_offset, int next_delay_pos)
{
  int data_in_first_block = block_size-byte_offset; //including bytes before start of data
  int data_in_second_block = std::min((int)(input_buffer_->size()-1), 1)*block_size;
  // Subtract one byte from available_data, because we always send an extra byte with each data packet
  int available_bytes = std::min(data_in_first_block+data_in_second_block-1, next_delay_pos);
  int nr_ffts = available_bytes/fftsize;

  // amount of available data
  int nbytes = std::min(nr_ffts*fftsize, INPUT_NODE_PACKET_SIZE-1);
  // The amount of data still to be send for this time slice
  int slice_left=data_writers_.front().slice_size;

  return std::min(nbytes,slice_left)+1;
}

void
Input_node_data_writer::init_random_block(std::vector<char> &data, int length) {
  if (data.size() != length) {
    data.resize(length);
#ifdef SFXC_INVALIDATE_SAMPLES
  #ifdef SFXC_CHECK_INVALID_SAMPLES
    for (int i=0; i<length; i++) {
      data[i] = INVALID_PATTERN;
    }
  #endif
#else
    // Randomize data
    for (int i=0; i<length; i++) {
      data[i] = (char)park_miller_random();
    }
#endif
  }
}

int64_t 
Input_node_data_writer::get_current_time(){
  return _current_time;
}
