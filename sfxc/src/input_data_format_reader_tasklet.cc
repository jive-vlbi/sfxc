#include "input_data_format_reader_tasklet.h"

Input_data_format_reader_tasklet::
Input_data_format_reader_tasklet(
  Data_format_reader_ptr reader,
  Input_memory_pool_ptr memory_pool,
  Data_frame &data)
    : memory_pool_(memory_pool),
      n_bytes_per_input_word(reader->bytes_per_input_word()),
      data_modulation(false) {

  SFXC_ASSERT(sizeof(value_type) == 1);
  output_buffer_ = Output_buffer_ptr(new Output_buffer());
  allocate_element();
  reader_ = reader;

  assert(data.buffer->data.size() > 0);

  input_element_ = data;
  current_time = reader_->get_current_time();
  push_element();

  data_read_=0;
}

Input_data_format_reader_tasklet::~Input_data_format_reader_tasklet(){  
}

void Input_data_format_reader_tasklet::stop() {
  isRunning = false;
  /// We add an empty interval to unblock the thread
  Time dummy;
  //dummy = (int64_t)0;
  add_time_interval(dummy, dummy);
}

void Input_data_format_reader_tasklet::do_execute() {
  isRunning = true;
  /// blocks until we have an interval to process
  fetch_next_time_interval();

  /// then let's work
  while ( isRunning ) {
    /// if there is still some data to process we do it
    if ( current_time < current_interval_.stop_time_ ){ 
      do_task();
    }/// otherwise we fetch a new interval.
    else fetch_next_time_interval();
  }
  DEBUG_MSG(" INPUT READER WILL EXIT ITS LOOP ");
  /// We close the queue in which we output our data.
  output_buffer_->close();
}

void
Input_data_format_reader_tasklet::
do_task() {
  // Compute the expected start time of the next frame
  Time start_next_frame = current_time + reader_->time_between_headers();
  allocate_element();

  if (reader_->eof()) {
    randomize_block();
  } else if (reader_->get_current_time() - current_time >= reader_->time_between_headers()) {
    randomize_block();
  } else {
    if (!reader_->read_new_block(input_element_)){
      randomize_block();
    }else if(reader_->get_current_time() != start_next_frame){
      int64_t nframes_missing = (reader_->get_current_time() - start_next_frame) / reader_->time_between_headers();
      int64_t nframes_left = (current_interval_.stop_time_ - start_next_frame) / reader_->time_between_headers();
      nframes_missing = std::min(nframes_missing, nframes_left);
      std::cout << RANK_OF_NODE << " : nframes_missing = " << nframes_missing << "; t= " << reader_->get_current_time() <<", expected="<<start_next_frame<<"\n";
      if(nframes_missing > 0){
        Input_element old_input_element = input_element_;
        for(int i=0; i < nframes_missing; i++){
          randomize_block();
          current_time += reader_->time_between_headers();
          input_element_.start_time = current_time;
          data_read_ += input_element_.buffer->data.size();
          push_element();
        }
        if(nframes_missing == nframes_left) // we are done, so don't insert last frame
          return;  
        input_element_ = old_input_element;
      } else if(nframes_missing < 0){
        do_task();
        return;
      }
    }
    if(data_modulation)
      demodulate(input_element_);
  }
  current_time += reader_->time_between_headers();
  input_element_.start_time = current_time;

  data_read_ += input_element_.buffer->data.size();

  push_element();
}

void
Input_data_format_reader_tasklet::fetch_next_time_interval() {
  /// Blocking function until a new interval is available
  current_interval_ = intervals_.front_and_pop();
  if ( !current_interval_.empty() ) {
    /// Otherwise the new interval is loaded.
    ///DEBUG_MSG(__PRETTY_FUNCTION__ << ":: SET TIME");
    ///DEBUG_MSG(__PRETTY_FUNCTION__ << ":: val:"<< current_interval_.start_time_ << " cur: "<< current_time);
    if(current_interval_.start_time_<=reader_->get_current_time()){
      current_time = reader_->get_current_time();
    }else{
      current_time = goto_time( current_interval_.start_time_);
      if (current_time > current_interval_.stop_time_) {
        current_time = current_interval_.stop_time_;
        randomize_block();
        input_element_.start_time = current_time;
      }
      data_read_ += input_element_.buffer->data.size();
      push_element();
    }
  }
}


void
Input_data_format_reader_tasklet::
add_time_interval(Time &start_time, Time &stop_time) {
  ///DEBUG_MSG("Time interval added: " << us_start_time << ":"<< us_stop_time );
  intervals_.push( Time_interval(start_time, stop_time ) );
}


bool
Input_data_format_reader_tasklet::
has_work() {
  if (memory_pool_->empty()) {
    return false;
  }

  return true;
}

void 
Input_data_format_reader_tasklet::set_parameters(const Input_node_parameters &input_node_param){
  data_modulation = input_node_param.data_modulation;
  reader_->set_parameters(input_node_param);
  int nchannel = input_node_param.channels.size();
  // For VDIF we assume that we have one thread per channel. We need a larger memory pool to ensure
  // that we can keep all the data writers busy.
  if(reader_->get_transport_type()==VDIF){
    memory_pool_->resize(20*nchannel);
  }
}

void
Input_data_format_reader_tasklet::
allocate_element() {
  input_element_.buffer = memory_pool_->allocate();
  input_element_.invalid.resize(0);
  input_element_.channel=0;
  input_element_.start_time=Time();
}

Time
Input_data_format_reader_tasklet::
goto_time(Time time) {

  Time new_time = reader_->goto_time(input_element_, time);
  SFXC_ASSERT(new_time == reader_->get_current_time());

  // Set the current time to the actual time in the data stream.
  // Might not be the requested time, if no data is available
  current_time = new_time;

  input_element_.start_time = current_time;

  if (time != new_time) {
    DEBUG_MSG("Warning: Couldn't go to time " << time << "us.");
    DEBUG_MSG("Current time is              " << current_time << "us.");
    DEBUG_MSG("Time found is                " << new_time << "us.");
  }

  return current_time;
}

Time
Input_data_format_reader_tasklet::
get_current_time() {
  return current_time;
}

void
Input_data_format_reader_tasklet::
push_element() {
  for(int i=0; i < input_element_.invalid.size(); i++){
    SFXC_ASSERT(input_element_.invalid[i].invalid_begin >= 0);
    SFXC_ASSERT(input_element_.invalid[i].nr_invalid >= 0);
  }
  SFXC_ASSERT(input_element_.buffer->data.size() == reader_->size_data_block());

  output_buffer_->push(input_element_);
}

Input_data_format_reader_tasklet::Output_buffer_ptr
Input_data_format_reader_tasklet::
get_output_buffer() {
  return output_buffer_;
}

std::vector< std::vector<int> >
Input_data_format_reader_tasklet::
get_tracks(const Input_node_parameters &input_node_param) {
  return reader_->get_tracks(input_node_param, input_element_);
}

void
Input_data_format_reader_tasklet::
randomize_block() {
  // Randomize/invalidate the data in the current block
  // Make sure the data has the right size
  size_t size = reader_->size_data_block();
  if (input_element_.buffer->data.size() != size) {
    input_element_.buffer->data.resize(size);
  }
  input_element_.channel = -1; // broadcast invalid data to all channels (VDIF ONLY)
#ifdef SFXC_INVALIDATE_SAMPLES
  input_element_.invalid.resize(1);
  input_element_.invalid[0].invalid_begin = 0;
  input_element_.invalid[0].nr_invalid = size;
#ifdef SFXC_CHECK_INVALID_SAMPLES
  value_type *buffer = input_element_.buffer->data[0];
  for (size_t i=0; i<size; i++) {
    buffer[i] = value_type(0);
  }
#endif // SFXC_CHECK_INVALID_SAMPLES

#else // !SFXC_INVALIDATE_SAMPLES
  value_type *buffer = input_element_.buffer->data[0];
  for (size_t i=0; i<size; i++) {
    // Randomize data
    // park_miller_random generates 31 random bits
    buffer[i] = (value_type)park_miller_random();
  }

#endif // SFXC_INVALIDATE_SAMPLES
}

void 
Input_data_format_reader_tasklet::demodulate(Input_element &data)
// See page 6 of Mark4 memo 230A, Whitney 2005
{
  std::vector<value_type> &buffer=data.buffer->data;
  int frame_size=buffer.size()/n_bytes_per_input_word;
  // The factor frame_size/8 is there because the sequence also advances at parity bits
  int sequence_length=frame_size+frame_size/8;

  if(demodulation_sequence.size()!=sequence_length){
    demodulation_sequence.resize(sequence_length); 
    gen_demodulation_sequence(sequence_length);
  }
  int index=0;
  for(int i=0;i<frame_size;i++){
    if(demodulation_sequence[i+i/8]!=0){
      for(int j=0;j<n_bytes_per_input_word;j++)
        buffer[index+j]=buffer[index+j]^0xff;
    }
    index+=n_bytes_per_input_word;
  }
}

void
Input_data_format_reader_tasklet::gen_demodulation_sequence(int sequence_length){
// See page 7 of Whitney 2005 (Mark4 memo 230a)
  unsigned int ret_val;
  unsigned int gen_sequence[16];
  //reset generator sequence
  for(int i=0;i<16;i++)
    gen_sequence[i]=1;

  for(int n=0;n<sequence_length;n++){
    ret_val = gen_sequence[10]^gen_sequence[12]^gen_sequence[13]^gen_sequence[15];
    for (int i=15;i>0;i--)
      gen_sequence[i]=gen_sequence[i-1];
    gen_sequence[0]=ret_val;
    demodulation_sequence[n]=(unsigned char)ret_val;
  }
}
