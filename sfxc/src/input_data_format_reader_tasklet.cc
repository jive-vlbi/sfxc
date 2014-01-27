#include "input_data_format_reader_tasklet.h"
#define NSKIP  16  // Number of frames to skip if we can't find a new header
#define NSKEW  128

Input_data_format_reader_tasklet::
Input_data_format_reader_tasklet(
  Data_format_reader_ptr reader,
  Input_memory_pool_ptr memory_pool)
    : memory_pool_(memory_pool),
      data_modulation(false), seqno(0) {

  SFXC_ASSERT(sizeof(value_type) == 1);
  output_buffer_ = Output_buffer_ptr(new Output_buffer());
  reader_ = reader;

  data_read_=0;
  allocate_element();
}

Input_data_format_reader_tasklet::~Input_data_format_reader_tasklet(){  
}

void Input_data_format_reader_tasklet::stop() {
  // An empty interval signals we're done
  Time dummy;
  add_time_interval(dummy, dummy);
}

void Input_data_format_reader_tasklet::do_execute() {
  // Block until we have an interval to process
  fetch_next_time_interval();
  // Do some work until we see an empty interval
  while (! current_interval_.empty()) {
    // As long as there still is data to process, do so
    if (*(std::min_element(current_time.begin(), current_time.end())) < current_interval_.stop_time_) {
      do_task();
    } else {
      fetch_next_time_interval();
    }
  }
  DEBUG_MSG(" INPUT READER WILL EXIT ITS LOOP ");
  output_buffer_->close();
}

void
Input_data_format_reader_tasklet::
do_task() {
  allocate_element();

  if (reader_->eof()){
    for (size_t i = 0; i < current_time.size(); i++){
      if (current_time[i] < current_interval_.stop_time_)
        push_random_blocks(1, i);
    }
    return;
  }
  for (size_t i = 0; i < current_time.size(); i++) {
    int nframes_left = (current_interval_.stop_time_ - current_time[i]) / reader_->time_between_headers();
    int skew = std::max(0, std::min(2 * NSKEW, nframes_left - 1));

    if (current_time[i] < max_time - reader_->time_between_headers() * skew) {
      int nframes = (max_time - current_time[i]) / reader_->time_between_headers() - skew;
      // Insert invalid blocks to prevent bad data streams to stall the correlation
      push_random_blocks(nframes, i);
      return;
    }
  }

  if (!reader_->read_new_block(input_element_)) {
    int nframes_left = (current_interval_.stop_time_ - current_time[0]) / reader_->time_between_headers();
    // Insert invalid blocks to prevent bad data streams to stall the correlation
    push_random_blocks(std::min(NSKIP, nframes_left), 0);
    return;
  }

  int channel = input_element_.channel;
  SFXC_ASSERT(channel >= 0 && channel < current_time.size());

  Time start_next_frame = current_time[channel];
  int nframes_left = (current_interval_.stop_time_ - start_next_frame) / reader_->time_between_headers();
  if (input_element_.start_time != start_next_frame) {
    int nframes_missing = (input_element_.start_time - start_next_frame) / reader_->time_between_headers();
    nframes_missing = std::min(nframes_missing, nframes_left);
#if 0
    std::cerr << RANK_OF_NODE << " : nframes_missing = " << nframes_missing << "; t= " << input_element_.start_time
	      << ", expected=" << start_next_frame << "\n";
#endif
    if (nframes_missing > 0) {
      time_t t;
      struct tm tm;
      ::time(&t);
      gmtime_r(&t, &tm);
      Time now(mjd(tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900),
	       tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec + 5);
      if (input_element_.start_time > now) {
	// Data from the future; drop the frame, insert an invalid block and complain
	std::cerr << RANK_OF_NODE << ": causality violation " << input_element_.start_time << std::endl;
	push_random_blocks(1, channel);
	return;
      }

      Input_element old_input_element = input_element_;
      push_random_blocks(nframes_missing, channel);
      input_element_ = old_input_element;
    } else if (nframes_missing < 0) {
      // Data was in the past; simply drop the frame
      return;
    }

    // We are done; don't insert the last frame
    if (nframes_missing == nframes_left)
      return;
  }

  if(input_element_.start_time != current_time[channel]){
    std::cerr.precision(16);
    std::cerr << RANK_OF_NODE << " : start_time = " << input_element_.start_time.get_time_usec()
              << ", current_time = " << current_time[channel].get_time_usec() << "\n";
  }
  SFXC_ASSERT(input_element_.start_time == current_time[channel]);

  if(data_modulation)
    demodulate(input_element_);

  data_read_ += input_element_.buffer->data.size();
  push_element();
  for (int i = 0; i < duplicate[channel].size(); i++) {
    input_element_.channel = duplicate[channel][i];
    push_element();
  }
}

void
Input_data_format_reader_tasklet::fetch_next_time_interval() {
  // Blocking function until a new interval is available
  current_interval_ = intervals_.front_and_pop();
  if (current_interval_.empty())
    return;

  if (!reader_->is_open() && !reader_->open_input_stream(input_element_)) {
    for (size_t i = 0; i < current_time.size(); i++)
      current_time[i] = current_interval_.start_time_;
    return;
  }

  SFXC_ASSERT(reader_->is_open());

  Time time;
  if (current_interval_.start_time_ <= input_element_.start_time) {
    // Ensure that the current_time is exactly at header_start, in mark5b sometimes the VLBA timestamp is used
    int64_t nframes = (int64_t)round((input_element_.start_time - current_interval_.start_time_) / reader_->time_between_headers());
    time = current_interval_.start_time_ + reader_->time_between_headers() * nframes;
  } else {
    time = goto_time(current_interval_.start_time_);
    int64_t nframes = (int64_t)round((time - current_interval_.start_time_)/ reader_->time_between_headers());
    time = current_interval_.start_time_ + reader_->time_between_headers() * nframes;
    if (time > current_interval_.stop_time_) {
      time = current_interval_.stop_time_ - reader_->time_between_headers();
      randomize_block();
      input_element_.start_time = time;
      input_element_.channel = 0;
    }else if (time < current_interval_.start_time_)
      time = current_interval_.start_time_;
  }

  for (size_t i = 0; i < current_time.size(); i++)
    current_time[i] = time;

  data_read_ += input_element_.buffer->data.size();
  push_element();
}

void
Input_data_format_reader_tasklet::push_random_blocks(int nblocks, int channel)
{
  for (int i = 0; i < nblocks; i++) {
    randomize_block();
    input_element_.start_time = current_time[channel];
    input_element_.channel = channel;
    data_read_ += input_element_.buffer->data.size();
    push_element();
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
Input_data_format_reader_tasklet::set_parameters(const Input_node_parameters &params){
  data_modulation = params.data_modulation;
  reader_->set_parameters(params);

  if (reader_->get_transport_type() == VDIF && params.n_tracks == 0) {
    current_time.resize(params.channels.size());

    // If we have multiple channels that are mapped to the same VDIF
    // thread, we will need to duplicate the data.  This typically
    // happens for mixed bandwidth correlations.
    duplicate.resize(params.channels.size());
    for (int i = 0; i < params.channels.size(); i++) {
      int thread = params.channels[i].tracks[0];
      for (int j = 0; j < params.channels.size(); j++) {
	if (j != i && params.channels[j].tracks[0] == thread)
	  duplicate[i].push_back(j);
      }
    }
  } else {
    current_time.resize(1);
    duplicate.resize(1);
  }

  for (size_t i = 0; i < current_time.size(); i++)
    current_time[i] = reader_->get_current_time();
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
  input_element_.start_time = new_time;

  if (time != new_time) {
    DEBUG_MSG("Warning: Couldn't go to time " << time << "us.");
    DEBUG_MSG("Current time is              " << current_time << "us.");
    DEBUG_MSG("Time found is                " << new_time << "us.");
  }

  return input_element_.start_time;
}

Time
Input_data_format_reader_tasklet::
get_current_time() {
  return current_time[0];
}

void
Input_data_format_reader_tasklet::
push_element() {
  for(int i=0; i < input_element_.invalid.size(); i++){
    SFXC_ASSERT(input_element_.invalid[i].invalid_begin >= 0);
    SFXC_ASSERT(input_element_.invalid[i].nr_invalid >= 0);
  }
  SFXC_ASSERT(input_element_.buffer->data.size() == reader_->size_data_block());
  input_element_.seqno = seqno++;
  current_time[input_element_.channel] += reader_->time_between_headers();
  if (current_time[input_element_.channel] > max_time)
    max_time = current_time[input_element_.channel];
  output_buffer_->push(input_element_);
}

Input_data_format_reader_tasklet::Output_buffer_ptr
Input_data_format_reader_tasklet::
get_output_buffer() {
  return output_buffer_;
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
  value_type *buffer = &input_element_.buffer->data[0];
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
  const size_t n_bytes_per_input_word = reader_->bytes_per_input_word();
          
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
