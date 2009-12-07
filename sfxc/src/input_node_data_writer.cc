#include <math.h>
#include <limits.h>
#include "input_node_data_writer.h"
Input_node_data_writer::
Input_node_data_writer(){
	last_duration_ = 0;
	total_data_written_ = 0;
  delay_index=0;
  _current_time=0;
  interval=0;
  sync_stream=false;
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
  // No data writers to send the data to
  if (data_writers_.empty())
    return false;

  // Check for new time interval
  if (!data_writers_.front().active){
    if ( _current_time >= current_interval_.stop_time_ ) {
      if (( intervals_.empty() ) || ( delays_.empty() )){
        return false;
      }
      interval++;
      fetch_next_time_interval();
    }
    // The data writer in the front of the queue is still being used
    // to send data from another channel
    //TODO check if it is possible that the same data_writer is used multiple times
    if (data_writers_.front().writer->is_active())
      return false;
  }

 // Not sufficient input data
  if(input_buffer_->empty())
    return false;

  // Check whether we can send data to the active writer
  if (!data_writers_.front().writer->can_write())
    return false;

  return true;
}

uint64_t
Input_node_data_writer::
do_task() {
  // Acquire the input data
  Input_buffer_element &input_element = input_buffer_->front();
  struct Writer_struct& data_writer = data_writers_.front();

  int64_t byte_offset=0;
  int samples_per_byte = 8/bits_per_sample;

  // Go to the current positition in the delay table
  std::vector<Delay> &cur_delay = delay_list.data();
  int delay_size = cur_delay.size();

  // Check whether we have to start a new timeslice
  if(!data_writer.active){
    // Initialise the size of the data slice
    SFXC_ASSERT(data_writer.slice_size>0);
    data_writer.writer->set_size_dataslice(-1);
    data_writer.writer->activate();
    data_writer.active = true;
    // Save the start time of the current slice, _current_time is calculated relative to this
    _slice_start = _current_time;
    samples_written_ = 0;

    DEBUG_MSG("FETCHING FOR A NEW WRITER......");
    sync_stream = true;
  }

  if(sync_stream){
    block_size=input_element.channel_data.data().data.size();
    int64_t dtime = (int64_t)(_current_time-input_element.start_time);
    byte_offset = dtime*sample_rate*bits_per_sample/8/1000000 + cur_delay[delay_index].bytes;

    if(byte_offset < 0){
      // The requested output lies (partly) before the input data, send invalid data
      int initial_delay = cur_delay[delay_index].remaining_samples;
      int64_t invalid_samples = write_initial_invalid_data(data_writer, byte_offset);
      data_writer.slice_size -= invalid_samples;
      samples_written_ += (invalid_samples-initial_delay);

      _current_time = _slice_start+(1000000LL*samples_written_+sample_rate/2)/sample_rate;
      sync_stream = false;
      return 0;
    }else if (byte_offset >= block_size){
      // This can happen if there is still a bit of old data left in the input queue
      input_buffer_->pop();
      return 0;
    }else{
      data_writer.slice_size += cur_delay[delay_index].remaining_samples;
      write_delay(data_writer.writer, cur_delay[delay_index].remaining_samples);
      samples_written_ -= cur_delay[delay_index].remaining_samples;
      sync_stream = false;
    }
  }
  Data_writer_sptr writer = data_writer.writer;

  // Check whether we have written all data to the data_writer
  if (data_writer.slice_size <= 0) {
      write_end_of_stream(data_writer.writer);
      // resync clock to a multiple of the integration time
      _current_time=((_current_time+integration_time/2)/integration_time)*integration_time;
      data_writer.writer->deactivate();
      data_writers_.pop();
      DEBUG_MSG("POPPING FOR A NEW WRITER......");
      return 0;
  }

  int index;
  int block_inv_samples=std::max((int64_t)0, input_element.nr_invalid_samples-byte_offset*samples_per_byte);
  if(block_inv_samples>0){
    write_invalid(writer, block_inv_samples);
    index = input_element.nr_invalid_samples/samples_per_byte; // We don't actually write the invalid bytes
  }else
    index=byte_offset;

  int next_delay_pos=get_next_delay_pos(cur_delay, _current_time)+byte_offset;
  int total_to_write = std::min(block_size-byte_offset,
                       (int64_t)(data_writer.slice_size+samples_per_byte-1)/samples_per_byte);
  int end_index = total_to_write+byte_offset;
  while(index<end_index){
    if(index>=next_delay_pos){
      delay_index++;

      write_delay(writer, cur_delay[delay_index].remaining_samples);
      // at delay change adjust the amount of samples to be sent
      int d_delay = cur_delay[delay_index].remaining_samples-cur_delay[delay_index-1].remaining_samples;
      if((d_delay>1)||(d_delay==-1))
        data_writer.slice_size -= 1;
      else 
        data_writer.slice_size += 1;
      next_delay_pos=get_next_delay_pos(cur_delay, _current_time)+byte_offset;
      total_to_write = std::min(block_size-byte_offset,
                       (int64_t)(data_writer.slice_size+samples_per_byte-1)/samples_per_byte);
      end_index = total_to_write+byte_offset;
    }else{
      int data_to_write = std::min(next_delay_pos-index, end_index-index);

      write_data(writer, data_to_write, index);
      index += data_to_write;
    }
  }
  total_data_written_ += total_to_write;
  data_writer.slice_size-=total_to_write*samples_per_byte;
  samples_written_ += total_to_write*samples_per_byte;

  _current_time = _slice_start+(1000000LL*samples_written_+sample_rate/2)/sample_rate;

  return total_to_write;
}

void
Input_node_data_writer::
add_timeslice(Data_writer_sptr data_writer, int64_t nr_samples) {
  Writer_struct writer;
  writer.writer = data_writer;
  writer.slice_size = nr_samples;

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
  time_per_byte=((8/bits_per_sample)*1000000.)/sample_rate;
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
Input_node_data_writer::write_invalid(Data_writer_sptr writer, int nInvalid){
  int8_t header = HEADER_INVALID;
  int bytes_written=0;

  while(bytes_written < nInvalid){
    // first write a header containing the number of bytes to be send
    int16_t data_to_write = (int16_t) std::min(nInvalid-bytes_written, SHRT_MAX);
    writer->put_bytes(sizeof(header), (char *)&header);
    writer->put_bytes(sizeof(data_to_write), (char *)&data_to_write);
    bytes_written += data_to_write;
  }
}

void
Input_node_data_writer::write_end_of_stream(Data_writer_sptr writer){
  int8_t header = HEADER_ENDSTREAM;
  writer->put_bytes(sizeof(header), (char *)&header);
}


void
Input_node_data_writer::write_delay(Data_writer_sptr writer, int8_t delay){
  // The header
  int8_t header_type = HEADER_DELAY;
  int nbytes = writer->put_bytes(sizeof(header_type), (char*)&header_type);
  SFXC_ASSERT(nbytes == sizeof(header_type));

  //The number delay in samples
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
  if(ndata==0)
    return;
  SFXC_ASSERT(byte_offset>=0);
  int bytes_written=0;
  int8_t header=HEADER_DATA;

  int start=byte_offset;

  while(bytes_written < ndata){
    // first write a header containing the number of bytes to be send
    int16_t data_to_write = (int16_t) std::min(ndata-bytes_written, SHRT_MAX);
    writer->put_bytes(sizeof(header), (char *)&header);
    writer->put_bytes(sizeof(data_to_write), (char *)&data_to_write);

    Input_buffer_element &input_element = input_buffer_->front();
    char *data =(char*)&input_element.channel_data.data().data[start];
    int written = 0;

    while(written < data_to_write){
      written += writer->put_bytes((int)data_to_write, data);
      bytes_written+=written;
      data+=written;
    }
    start += data_to_write;
  }
  // If we are at the end of the input buffer remove it from the queue
  if((bytes_written+byte_offset)%block_size==0){
    input_buffer_->pop();
  }
}

int64_t 
Input_node_data_writer::write_initial_invalid_data(Writer_struct &data_writer, int byte_offset){
  int samples_per_byte = 8/bits_per_sample;
  std::vector<Delay> &cur_delay = delay_list.data();
  int delay_size = cur_delay.size();

  // The initial delay
  write_delay(data_writer.writer, cur_delay[delay_index].remaining_samples);
  data_writer.slice_size += cur_delay[delay_index].remaining_samples;

  int64_t invalid_samples=std::min((int64_t)-byte_offset*samples_per_byte, data_writer.slice_size);
  int64_t written=0;
  while(written<invalid_samples){
    int64_t next_delay_pos;
    if(delay_index<delay_size-1){
      int64_t dt=(cur_delay[delay_index+1].time-_current_time);
      next_delay_pos=dt*sample_rate/1000000LL;
    }else{
      next_delay_pos=data_writer.slice_size*2;
    }
    if(written==next_delay_pos){
      delay_index++;
      write_delay(data_writer.writer, cur_delay[delay_index].remaining_samples);
      // at delay change adjust the amount of samples to be sent
      int d_delay = cur_delay[delay_index].remaining_samples-cur_delay[delay_index-1].remaining_samples;
      if((d_delay>1)||(d_delay==-1))
        data_writer.slice_size -= 1;
      else
        data_writer.slice_size += 1;
      invalid_samples=std::min((int64_t)-byte_offset*samples_per_byte, data_writer.slice_size);
    }else{
      int data_to_write = std::min(next_delay_pos-written, invalid_samples-written);
      write_invalid(data_writer.writer, data_to_write);
      written += data_to_write;
    }
  }
  return invalid_samples;
}

int64_t 
Input_node_data_writer::get_current_time(){
  return _current_time;
}
